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

    }

    void OutputBucket::setBin(std::weak_ptr<Bin> bin)
    {
        Bucket::setBin(bin);

        if(!isEmpty())
        {
            auto b = getBin();
            uint8_t item = b->getItemId();
            //TODO WEIGHT UPDATE
            getBin()->setAmount(10);
        }
    }

    bool Storage::move(Bucket* src, Bucket* dst,bool force)
    {
        if(!src || !dst || src==dst)
            return true;
        mov_control.platform.move(controls::PlatformControl::Direction::RETRACT);
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
            }
        }
        rfid_t rfid;
        mov_control.xy_table.moveTo(src->getPos(),controls::Speed::FAST);
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
        }
        return target;

    }

    Storage::OperationStatus Storage::map()
    {
        mov_control.xy_table.calibrate();
        for(auto bucket : buckets)
            readBucket(&bucket);
        readBucket(&interface_bucket);
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
        if(!interface_bucket.isEmpty())
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
        Bucket* buck = getEmptyBucket(bin->getUses());
        if(buck == nullptr)
        {
            map();
            buck = getEmptyBucket(bin->getUses());
            if(buck == nullptr)
                return ERROR_FULL;
        }

        if(!move(&interface_bucket,buck))
        {
            map();
            buck = getEmptyBucket(bin->getUses());
            if(buck == nullptr || !move(&interface_bucket,buck))
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

    Storage::OperationStatus Storage::reorganize()
    {
        //TODO
        return OK;
    }

    Bucket* Storage::getEmptyBucket(int uses)
    {
        //TODO MAKE IT GET IN CONSIDERATION USES TO PUT IT CLOSER TO THE POSITION IT NEEDS TO BE
        for(auto & b : buckets)
        {
            if(b.isEmpty())
                return &b;
        }
        return nullptr;
    }

    bool Storage::needsReorganizing()
    {
        //analyze buckets to see if it needs reorganization;
        return false;
    }
    void Storage::init()
    {
        SPIFFS.begin(true,"/spiffs");
        //start spiffs
        int offset_x=0, offset_y=0;
        int x_positions[] = {20,332,635,938,1243};
        int y_positions[] = {20,400,700,1000,1300,1600};

        buckets[0] = Bucket({x_positions[2],y_positions[5]});
        buckets[1] = Bucket({x_positions[3],y_positions[5]});
        buckets[2] = Bucket({x_positions[4],y_positions[5]});
        buckets[3] = Bucket({x_positions[4],y_positions[4]});
        buckets[4] = Bucket({x_positions[3],y_positions[4]});
        buckets[5] = Bucket({x_positions[2],y_positions[4]});
        buckets[6] = Bucket({x_positions[2],y_positions[3]});
        buckets[7] = Bucket({x_positions[3],y_positions[3]});
        buckets[8] = Bucket({x_positions[4],y_positions[3]});
        buckets[9] = Bucket({x_positions[4],y_positions[2]});
        buckets[10] = Bucket({x_positions[3],y_positions[2]});
        buckets[11] = Bucket({x_positions[2],y_positions[2]});
        buckets[12] = Bucket({x_positions[1],y_positions[2]});
        buckets[13] = Bucket({x_positions[0],y_positions[2]});
        buckets[14] = Bucket({x_positions[0],y_positions[1]});
        buckets[15] = Bucket({x_positions[1],y_positions[1]});
        buckets[16] = Bucket({x_positions[2],y_positions[1]});
        buckets[17] = Bucket({x_positions[3],y_positions[1]});
        buckets[18] = Bucket({x_positions[4],y_positions[1]});
        buckets[19] =  Bucket({x_positions[4],y_positions[0]});
        buckets[20] = Bucket({x_positions[3],y_positions[0]});
        buckets[21] = Bucket({x_positions[2],y_positions[0]});
        buckets[22] = Bucket({x_positions[1],y_positions[0]});
        buckets[23] = Bucket({x_positions[0],y_positions[0]});
        interface_bucket = OutputBucket({20,1400});

        desserialize();
        mov_control.init();
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

        while(file.read(reinterpret_cast<char*>(rfid.data()),sizeof(rfid))
            && file.read(reinterpret_cast<char*>(&id),sizeof(id))
            && file.read(reinterpret_cast<char*>(&amount),sizeof(amount))
            && file.read(reinterpret_cast<char*>(&uses),sizeof(uses)))
        {
            bins.push_back(std::make_shared<Bin>(rfid,amount,id,uses));
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
            int uses = b->getUses();
            file.write(reinterpret_cast<const char*>(rfid.data()),sizeof(rfid));
            file.write(reinterpret_cast<const char*>(&id),sizeof(id));
            file.write(reinterpret_cast<const char*>(&amount),sizeof(amount));
            file.write(reinterpret_cast<const char*>(&uses),sizeof(uses));
        }

        
    }

    void Storage::getJson(String& json)
    {
        json = "{ \"bins\": [";
        bool first = true;
        int i = 0;
        for(auto& buck : buckets)
        {
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
            i++;
            json += "\",";
            json += "\"rfid\":\"";
            for(auto byte : b->getRFID())
            {
                char buf[3];
                sprintf(buf,"%02X",byte);
                json += buf;
            }
            json += ",";
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
                json += String(i);
                i++;
                json += "\"rfid\":\"";
                for(auto byte : b->getRFID())
                {
                    char buf[3];
                    sprintf(buf,"%02X",byte);
                    json += buf;
                }
                json += ",";
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
}