#pragma once
#include "Controls.h"
#include <memory>
#include <array>
#include <vector>

namespace storage
{
    
using rfid_t = controls::rfid_t;

class Bin
{
    rfid_t rfid;
    int quantity;
    uint8_t item_id;
    int uses;

    public:

    int getAmount();
    uint8_t getItemId();
    int getUses();
    rfid_t getRFID();
    void setRFID(rfid_t& rfid);
    void setItemId(uint8_t id);
    void setUses(int uses);
    void setAmount(int amount);
    Bin(rfid_t rfid,int amount, int item_id,int uses);

};

class Bucket
{
    protected:
    controls::Pos2i position;
    std::weak_ptr<Bin> cur_bin;

    public:

    Bucket(controls::Pos2i pos,std::weak_ptr<Bin> bin=std::weak_ptr<Bin>{});

    virtual void setBin(std::weak_ptr<Bin> bin=std::weak_ptr<Bin>{});
    virtual bool isEmpty();

    std::shared_ptr<Bin> getBin();
    controls::Pos2i getPos();

};

class OutputBucket: public Bucket {
    public:
    OutputBucket(controls::Pos2i pos,std::weak_ptr<Bin> bin=std::weak_ptr<Bin>{});
    void setBin(std::weak_ptr<Bin> bin=std::weak_ptr<Bin>{});
};


class Storage {
    std::array<Bucket,24> buckets;
    OutputBucket interface_bucket;
    std::vector<std::shared_ptr<Bin>> bins;
    controls::MovementControl mov_control;

    bool move(Bucket* src, Bucket* dst,bool force = false);
    void desserialize();
    void serialize();
    public:

    enum OperationStatus
    {
        OK,
        OK_NEEDS_REORGANIZING,
        ERROR_OUTPUT_EMPTY,
        ERROR_OUTPUT_NOT_EMPTY,
        ERROR_BIN_NOT_FOUND,
        ERROR_FULL,
    };
    OperationStatus map();
    OperationStatus store(bool change_id=false,uint8_t item_id=0); //serialize after reading the weight
    OperationStatus retrieve(rfid_t& rfid); //serialize at the end
    OperationStatus reorganize();
    Bucket* readBucketFromId(int id);
    Bucket* readBucket(Bucket* bucket);
    Bucket* getBucketByRfid(rfid_t& rfid);
    Bucket* getEmptyBucket(int uses);
    std::weak_ptr<Bin> getBinByRfid(rfid_t& rfid);
    std::vector<Bucket*> getBucketByType(uint8_t type_id);
    bool needsReorganizing();


    void init();
};
}