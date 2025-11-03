#include "Storage.h"
#include <fstream>
#include <SPIFFS.h> 
namespace storage
{
    int Bin::getAmount()
    {
        return quantity;
    }

    uint8_t Bin::getItemId()
    {
        return item_id;
    }

    rfid_t Bin::getRFID()
    {
        return rfid;
    }

    int Bin::getUses()
    {
        return uses;
    }

    void Bin::setRFID(rfid_t& rfid)
    {
        this->rfid = rfid;
    }

    void Bin::setItemId(uint8_t id)
    {
        item_id = id;
    }

    void Bin::setUses(int uses)
    {
        this->uses = uses;
    }

    void Bin::setAmount(int amount)
    {
        this->quantity = amount;
    }

    Bin::Bin(rfid_t rfid,int amount, int item_id,int uses)
    : rfid(rfid), quantity(amount),item_id(item_id),uses(uses)
    {

    }

    Bucket::Bucket(controls::Pos2i pos,std::weak_ptr<Bin> bin)
    :position(pos),cur_bin(bin)
    {

    }

    void Bucket::setBin(std::weak_ptr<Bin> bin)
    {
        cur_bin = bin;
    }

    bool Bucket::isEmpty()
    {
        return cur_bin.expired();
    }

    std::shared_ptr<Bin> Bucket::getBin()
    {
        return cur_bin.lock();
    }

    controls::Pos2i Bucket::getPos()
    {
        return position;
    }

    OutputBucket::OutputBucket(controls::Pos2i pos,std::weak_ptr<Bin> bin)
    :Bucket(pos,bin)
    {
        hx711.begin(DT_PIN,SCK_PIN);
        hx711.set_scale(SCALE);
        delay(1000);
        hx711.wait_ready(1000);
        hx711.tare();
    }

    float OutputBucket::updateWeight(uint8_t type)
    {
        if(isEmpty())
            return 0;
        delay(1000);
        hx711.wait_ready(200);
        float weight = hx711.get_units(10)-BIN_WEIGHT;
        float item_weight = Item::getWeight(type);
        if(weight<1.2f)
        {
            getBin()->setAmount(0);
        }else
        {
            
            getBin()->setAmount((weight+0.5*item_weight)/item_weight);
        }

        if(getBin()->getAmount()<=0)
            getBin()->setItemId(0);

        printf("WEIGHT UPDATED TO: %f g\n",weight);
        return weight;

    }

    void OutputBucket::tare()
    {
        delay(1000);
        hx711.wait_ready(1000);
        hx711.tare();
    }

    void OutputBucket::setBin(std::weak_ptr<Bin> bin)
    {
        Bucket::setBin(bin);

        if(!isEmpty())
        {
            auto b = getBin();
            uint8_t item = b->getItemId();
            float weight = updateWeight(item);
        }
    }

    bool Storage::move(Bucket* src, Bucket* dst,bool force)
    {
        if(!src || !dst || src==dst)
            return true;
        if(!force)
        {
            rfid_t rfid;
            mov_control.xy_table.moveTo(dst->getPos(),controls::Speed::FAST);
            if(mov_control.read(rfid))
            {
                Bucket* buck = getBucketByRfid(rfid);
                if(buck)
                {
                    auto bin = buck->getBin();
                    buck->setBin();
                    dst->setBin(bin);
                } else {
                    std::weak_ptr<Bin> bin = getBinByRfid(rfid);
                    if(bin.expired())
                    {
                        auto new_bin = std::make_shared<Bin>(rfid,0,0,0);
                        bins.push_back(new_bin);
                        dst->setBin(new_bin);
                    }else
                    {
                        dst->setBin(bin);
                    }
                }
                return false;
            } else
            {
                dst->setBin();
            }
        }
        rfid_t rfid;
        mov_control.xy_table.moveTo(src->getPos(),controls::Speed::FAST);
        if(mov_control.xy_table.getPos() == src->getPos())
            delay(500);
        if(!mov_control.readAndFetch(rfid))
            return true;
        mov_control.xy_table.moveTo(dst->getPos(),controls::Speed::MEDIUM);
        mov_control.store();

        Bucket* buck = getBucketByRfid(rfid);
        if(buck)
        {
            auto bin = buck->getBin();
            buck->setBin();
            src->setBin();
            dst->setBin(bin);
        } else {
            std::weak_ptr<Bin> bin = getBinByRfid(rfid);
            if(bin.expired())
            {
                auto new_bin = std::make_shared<Bin>(rfid,0,0,0);
                bins.push_back(new_bin);
                dst->setBin(new_bin);
            }else
            {
                dst->setBin(bin);
            }
            src->setBin();
        }
        return true;

    }
    Bucket* Storage::readBucketFromId(int id)
    {
        Bucket* target = (id<0 || id>=buckets.size())? &interface_bucket : &buckets[id];
        return readBucket(target);
    }

    Bucket* Storage::readBucket(Bucket* target)
    {
        
        mov_control.platform.move(controls::PlatformControl::Direction::RETRACT);
        rfid_t rfid;
        mov_control.xy_table.moveTo(target->getPos(),controls::Speed::FAST);
        if(mov_control.read(rfid))
        {
            Bucket* buck = getBucketByRfid(rfid);
            if(buck)
            {
                auto bin = buck->getBin();
                buck->setBin();
                target->setBin(bin);
            } else {
                std::weak_ptr<Bin> bin = getBinByRfid(rfid);
                if(bin.expired())
                {
                    auto new_bin = std::make_shared<Bin>(rfid,0,0,0);
                    bins.push_back(new_bin);
                    target->setBin(new_bin);
                }else
                {
                    target->setBin(bin);
                }
            }
        } else 
        {
            target->setBin();
        }
        return target;

    }

    Storage::OperationStatus Storage::map()
    {
        mov_control.xy_table.calibrate();
        for(auto& bucket : buckets)
            readBucket(&bucket);
        readBucket(&interface_bucket);
        mov_control.xy_table.calibrate();
        serialize();
        return OperationStatus::OK;
    }

    Bucket* Storage::getBucketByRfid(rfid_t& rfid)
    {
        for(auto& b : buckets)
        {
            if(!b.isEmpty())
            {
                if(b.getBin()->getRFID() == rfid)
                    return &b;
            }
        }
        if(!interface_bucket.isEmpty())
        {
            if(interface_bucket.getBin()->getRFID() == rfid)
                    return &interface_bucket;
        }
        return nullptr;
    }

    std::vector<Bucket*> Storage::getBucketByType(uint8_t type)
    {
        std::vector<Bucket*> bucks{};
        for(auto& b : buckets)
        {
            if(!b.isEmpty())
            {
                if(b.getBin()->getItemId() == type)
                    bucks.push_back(&b);
            }
        }
        if(!interface_bucket.isEmpty() &&interface_bucket.getBin()->getItemId() == type)
        {
            bucks.push_back(&interface_bucket);
        }
        return bucks;
    }

    std::weak_ptr<Bin> Storage::getBinByRfid(rfid_t& rfid)
    {
        for(auto b: bins)
        {
            if(b->getRFID() == rfid)
                return b;
        }
        return std::weak_ptr<Bin>{};
    }

    Storage::OperationStatus Storage::store(bool change_id,uint8_t item_id)
    {
        readBucket(&interface_bucket);
        if(interface_bucket.isEmpty())
            return ERROR_OUTPUT_EMPTY;
        std::shared_ptr<Bin> bin = interface_bucket.getBin();
        if(change_id)
        {
            bin->setUses(0);
            bin->setItemId(item_id);
        }
        interface_bucket.setBin(bin);
        serialize();
        auto empty_bucks = getEmptyBucket(bin->getUses());
        Bucket* buck = nullptr;
        for(auto b : empty_bucks)
        {
            if(readBucket(b)->isEmpty())
            {
                buck = b;
                break;
            }
        }
        if(buck == nullptr)
        {
            map();
            auto empty_bucks = getEmptyBucket(bin->getUses());
            if(empty_bucks.empty() || !move(&interface_bucket,empty_bucks[0]))
                return ERROR_FULL;
        }
        

        if(!move(&interface_bucket,buck,true))
        {
            map();
            auto empty_bucks = getEmptyBucket(bin->getUses());
            if(empty_bucks.empty() || !move(&interface_bucket,empty_bucks[0]))
                return ERROR_FULL;    
               
        }
        
        if(needsReorganizing())
            return OK_NEEDS_REORGANIZING;
        return OK;
    }
    Storage::OperationStatus Storage::retrieve(rfid_t& rfid)
    {
        Bucket* buck = getBucketByRfid(rfid);
        if(!readBucket(&interface_bucket)->isEmpty())
        {
            return ERROR_OUTPUT_NOT_EMPTY;
        }

        if(!buck)
        {
            map();
            buck = getBucketByRfid(rfid);
            if(!buck)
                return ERROR_BIN_NOT_FOUND;
        } else 
        {
           buck =  readBucket(buck);
           if(buck->isEmpty() || buck->getBin()->getRFID()!=rfid)
           {
                map();
                buck = getBucketByRfid(rfid);
                if(!buck)
                    return ERROR_BIN_NOT_FOUND;
           }
        }
        move(buck,&interface_bucket,true);
        serialize();
        if(!interface_bucket.isEmpty())
        {
            auto bin = interface_bucket.getBin();
            bin->setUses(bin->getUses()+1);
        }
        //check if needs reorganizing
        if(needsReorganizing())
            return OK_NEEDS_REORGANIZING;
        return OK;
    }
    std::list<int> Storage::organized_storage()
    {
        printf("b1\n");
        std::list<int> all_bins;
        std::list<int> current_bins;
        all_bins.clear();
        current_bins.clear();
        printf("b2\n");
        for(auto& el : bins)
        {
                all_bins.push_back(el->getUses());  
        }
        printf("b3\n");
        for(auto& el : buckets)
        {
            if(!el.isEmpty())
                current_bins.push_back(el.getBin()->getUses());
        }
        printf("b4\n");
        if(!interface_bucket.isEmpty())
        {
            current_bins.push_back(interface_bucket.getBin()->getUses());
        }
        printf("b5\n");
        
        current_bins.sort([](int a, int b) 
            { return a < b;});
        printf("b6\n");
        all_bins.sort([](int a, int b) 
            { return a < b;});
        printf("b7\n");


        auto cur_it = current_bins.begin();
        printf("b7.5\n");
        auto all_it = all_bins.begin();
        printf("b8\n");
        if(current_bins.size() > buckets.size())
        {
            current_bins.reverse();
            return current_bins;
        }
        printf("b9\n");
        //remove the binspaces for empty
        while(all_bins.size() > buckets.size())
        {
            printf("b10\n");
            if(*all_it != *cur_it )
            {
                all_it = all_bins.erase(all_it);
            } else
            {
                printf("b11\n");
                all_it++;
                cur_it++;
            }
        }
        all_bins.reverse();
        //push_back use 0 until bins has the same size as buckets
        while(all_bins.size()<buckets.size())
            all_bins.push_back(0);

        return all_bins;
    }
    std::list<Bucket*> Storage::organize_bucket_order()
    {
        printf("a1\n");
        std::list<Bucket*> all_buckets;
        all_buckets.clear();
        printf("a2\n");
        for(auto& el : buckets)
        {
            all_buckets.push_back(&el);
        }
        printf("a3\n");
        OutputBucket* inter = &interface_bucket;
        printf("a4\n");
        all_buckets.sort([inter](Bucket* a, Bucket* b) 
            { return controls::Pos2i::distance(a->getPos(),inter->getPos()) < controls::Pos2i::distance(b->getPos(),inter->getPos());});
        printf("a5\n");
        return all_buckets;
    }
    Storage::OperationStatus Storage::reorganize(bool reweight)
    {
        map();
        store();
        
        auto bucks_org = organize_bucket_order();
        auto bins_org = organized_storage();
        if(bins_org.size() > buckets.size())
            return ERROR_FULL;
        auto bu = bucks_org.begin();
        auto bi = bins_org.begin();

        while(bu != bucks_org.end() && bi != bins_org.end())
        {

            if(!(*bu)->isEmpty() && (*bu)->getBin()->getUses() == *bi && !reweight)
            {
                bu++;
                bi++;
                continue;
            }
            //find first bucket with a bin with that given uses
            Bucket* src = nullptr;
            for(auto it = bu; it != bucks_org.end(); it++)
            {
                if(!(*it)->isEmpty() && (*it)->getBin()->getUses() == *bi)
                {
                    src = *it;
                    break;
                }
            }
            if(src)
            {
                move(src,&interface_bucket,true);
                if(!(*bu)->isEmpty())
                    move(*bu,src,true);
                move(&interface_bucket,*bu,true);
            }
            bu++;
            bi++;
        }
        
        
        return OK;
    }

    std::vector<Bucket*> Storage::getEmptyBucket(int uses)
    {
        //TODO MAKE IT GET IN CONSIDERATION USES TO PUT IT CLOSER TO THE POSITION IT NEEDS TO BE

        //Create vector with a relation bucket x use it should have, only for empty buckets
        auto bucks_org = organize_bucket_order();
        auto bins_org = organized_storage();
        struct buck_val
        {
            Bucket* buck;
            int uses;
        };
        std::vector<buck_val> bucks;
        auto bu = bucks_org.begin();
        auto bi = bins_org.begin();
        while(bu!= bucks_org.end() && bi!= bins_org.end())
        {
            if((*bu)->isEmpty())
                bucks.push_back(buck_val{*bu,*bi});
            bu++;
            bi++;
        }
        //sort it by the distance its uses has from the favored use
        std::stable_sort(bucks.begin(),bucks.end(),[uses](const buck_val& a,const buck_val& b){
            return abs(a.uses-uses) < abs(b.uses-uses);
        });

        std::vector<Bucket*> empty_buckets;
        for(auto& el : bucks)
        {
            empty_buckets.push_back(el.buck);
        }

        return empty_buckets;
    }

    bool Storage::needsReorganizing()
    {
        auto bins_org = organized_storage();
        auto bucks_org = organize_bucket_order();

        if(bins_org.size()> buckets.size())
            return false;
        auto bu = bucks_org.begin();
        auto bi = bins_org.begin();

        while(bu != bucks_org.end() && bi != bins_org.end())
        {
            if(!(*bu)->isEmpty() && (*bu)->getBin()->getUses() != *bi)
            {
                return true;
            }
            bu++;
            bi++;
        }
        //analyze buckets to see if it needs reorganization;
        return false;
    }
    void Storage::init()
    {
        SPIFFS.begin(true,"/spiffs");
        //start spiffs
        int offset_x=0, offset_y=0;
        int x_positions[] = {70,360,660,960,1260};
        int y_positions[] = {0,346,680,1030,1350,1691};

        buckets[0] = Bucket({647,y_positions[5]});
        buckets[1] = Bucket({940,y_positions[5]});
        buckets[2] = Bucket({1240,y_positions[5]});
        buckets[3] = Bucket({1240,y_positions[4]});
        buckets[4] = Bucket({950,y_positions[4]});
        buckets[5] = Bucket({650,y_positions[4]});
        buckets[6] = Bucket({646,y_positions[3]});
        buckets[7] = Bucket({946,y_positions[3]});
        buckets[8] = Bucket({1250,y_positions[3]});


        buckets[9] = Bucket({1270,y_positions[2]});
        buckets[10] = Bucket({970,y_positions[2]});
        buckets[11] = Bucket({665,y_positions[2]});
        buckets[12] = Bucket({355,y_positions[2]});
        buckets[13] = Bucket({55,690});
        buckets[14] = Bucket({70,y_positions[1]});
        buckets[15] = Bucket({355,y_positions[1]});
        buckets[16] = Bucket({665,y_positions[1]});
        buckets[17] = Bucket({970,y_positions[1]});
        buckets[18] = Bucket({1260,y_positions[1]});
        buckets[19] =  Bucket({1270,y_positions[0]});
        buckets[20] = Bucket({970,y_positions[0]});
        buckets[21] = Bucket({663,y_positions[0]});
        buckets[22] = Bucket({363,y_positions[0]});
        buckets[23] = Bucket({70,y_positions[0]});
        interface_bucket = OutputBucket({30,1350});

        desserialize();
        mov_control.init();
        mov_control.xy_table.moveTo(interface_bucket.getPos(),controls::Speed::FAST);
        rfid_t rfid;
        bool found_bin = mov_control.readAndFetch(rfid);
        interface_bucket.tare();
        if(found_bin)
            mov_control.store();
        //map();
        
    }

    void Storage::desserialize()
    {
        std::ifstream file("/spiffs/bins",std::ios::binary);
        if(!file)
            return;

        bins.clear();
        bins.reserve(24);

        rfid_t rfid;
        uint8_t id;
        int amount;
        int uses;
        int pos;

        while(file.read(reinterpret_cast<char*>(rfid.data()),sizeof(rfid))
            && file.read(reinterpret_cast<char*>(&id),sizeof(id))
            && file.read(reinterpret_cast<char*>(&amount),sizeof(amount))
            && file.read(reinterpret_cast<char*>(&uses),sizeof(uses))
            && file.read(reinterpret_cast<char*>(&pos),sizeof(pos)))
        {
            bins.push_back(std::make_shared<Bin>(rfid,amount,id,uses));
            if(pos<buckets.size())
            {
                buckets[pos].setBin(bins.back());
            }
        }
    }
    void Storage::serialize()
    {
        std::ofstream file("/spiffs/bins",std::ios::binary);
        if(!file)
            return;
        for(auto& b : bins)
        {
            rfid_t rfid = b->getRFID();
            uint8_t id = b->getItemId();
            int amount = b->getAmount();
            int i;
            for(i =0; i<buckets.size();i++)
            {
                if(buckets[i].getBin() == b)
                    break;
            }
            int uses = b->getUses();
            file.write(reinterpret_cast<const char*>(rfid.data()),sizeof(rfid));
            file.write(reinterpret_cast<const char*>(&id),sizeof(id));
            file.write(reinterpret_cast<const char*>(&amount),sizeof(amount));
            file.write(reinterpret_cast<const char*>(&uses),sizeof(uses));
            file.write(reinterpret_cast<const char*>(&i),sizeof(i));
        }

        
    }

    void Storage::getJson(String& json)
    {
        json = "{ \"bins\": [";
        bool first = true;
        
        for(int i =0; i<buckets.size();i++)
        {
            auto& buck = buckets[i];
            if(buck.isEmpty())
                continue;
            if(!first)
                json += ",";
            first = false;
            auto b = buck.getBin();
            String item_name = Item::getName(b->getItemId());
            json += "{";
            json += "\"position\":";
            json += String(i);
            json += ",";
            json += "\"rfid\":\"";
            for(auto byte : b->getRFID())
            {
                char buf[3];
                sprintf(buf,"%02X",byte);
                json += buf;
            }
            json += "\",";
            json += "\"item_name\":\"";
            json += item_name;
            json += "\",";
            json += "\"amount\":";
            json += String(b->getAmount());
            json += ",";
            json += "\"uses\":";
            json += String(b->getUses());
            json += "}";
        }


            if(!interface_bucket.isEmpty())
            {
                if(!first)
                    json += ",";
                first = false;
                auto b = interface_bucket.getBin();
                String item_name = Item::getName(b->getItemId());
                json += "{";
                json += "\"position\":";
                json += String(255);
                json += ",\"rfid\":\"";
                for(auto byte : b->getRFID())
                {
                    char buf[3];
                    sprintf(buf,"%02X",byte);
                    json += buf;
                }
                json += "\",";
                json += "\"item_name\":\"";
                json += item_name;
                json += "\",";
                json += "\"amount\":";
                json += String(b->getAmount());
                json += ",";
                json += "\"uses\":";
                json += String(b->getUses());
                json += "}";
            }

        json += "] }";

    }

    Bucket* Storage::FindFittingBucket(int amount, uint8_t type)
    {
        auto bucks = getBucketByType(type);
        if(!bucks.empty())
        {
            for(auto* b : bucks)
            {
                if(b!=&interface_bucket && b->getBin()->getAmount()+amount <= Item::getMaxAmount(type))
                {
                    return b;
                }
            }
        }

        auto other_bucks = getBucketByType(0);
        if(other_bucks.empty())
            return nullptr;
        return other_bucks[0];
    }
}


