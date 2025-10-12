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

    bool Storage::move(Bucket* src, Bucket* dst,bool force = false)
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
        mov_control.xy_table.moveTo(src->getPos());
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
        for(auto bucket : buckets)
            readBucket(&bucket);
        readBucket(&interface_bucket);
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
        //see if needs reorganizing, with OK;
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
    }

    bool Storage::needsReorganizing()
    {
        //analyze buckets to see if it needs reorganization;
        return false;
    }
    void Storage::init()
    {
        SPIFFS.begin(true,"/data");
        //start spiffs
        //set all buckets,
        desserialize();
        mov_control.init();
        map();
    }

    void Storage::desserialize()
    {
        std::ifstream file("/data/bins",std::ios::binary);
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
        std::ofstream file("/data/bins",std::ios::binary);
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
}