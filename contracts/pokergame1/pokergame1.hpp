//
// Created by Kevin Huang on 7/20/18.
//

#ifndef PROJECT_POKERGAME1_H
#define PROJECT_POKERGAME1_H
#endif

//#include <math.h>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/currency.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/transaction.hpp>
#include <set>
#include <sstream>
#include <unordered_map>

using namespace eosio;
using std::sort;
using std::string;
using std::stringstream;
using std::hash;
using std::unordered_map;
using std::make_pair;


class pokergame1 : public eosio::contract {

public:
    pokergame1(account_name self)
            :contract(self),
             pools(_self, _self),
             events(_self, _self),
             metadatas(_self, _self),
             secrets(_self, _self){};
    //@abi action
    void dealreceipt(const name from, string hash1, string hash2, string card1, string card2, string card3, string card4, string card5, uint64_t bet, uint64_t win);
    //@abi action
    void drawcards(const name from, uint32_t externalsrc, string dump1, string dump2, string dump3, string dump4, string dump5);
    //@abi action
    void clear();
    //@abi action
    void setcards(const name from, uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t c5);
    //@abi action
    void setseed(const name from, uint32_t seed);
    checksum256 gethash(account_name from);
    uint32_t getcard(account_name from, checksum256 result);
    void deposit(const currency::transfer &t, account_name code);
    bool checkflush(uint32_t colors[5]);
    bool checkstraight(uint32_t numbers[5]);
    uint32_t checksame(uint32_t numbers[5], uint32_t threshold);
    bool ishold(string s);
    bool checkBiggerJack(uint32_t numbers[5]);
    uint32_t parsecard(string s);


private:
    // @abi table metadatas i64
    struct st_metadatas {
        uint64_t id;
        uint32_t eventcnt;
        uint32_t idx;
        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(st_metadatas, (id)(eventcnt)(idx))
    };

    // @abi table pools i64
    struct st_pools {
        name owner;
        uint32_t card1;
        uint32_t card2;
        uint32_t card3;
        uint32_t card4;
        uint32_t card5;
        uint32_t wintype;
        uint64_t bet;
        uint64_t betwin;
        uint64_t userseed;
        string cardhash1;
        string cardhash2;

        uint64_t primary_key() const { return owner; }

        EOSLIB_SERIALIZE(st_pools, (owner)(card1)(card2)(card3)(card4)(card5)(wintype)(bet)(betwin)(userseed)(cardhash1)(cardhash2))
    };

    // @abi table events i64
    struct st_events {
        uint64_t id;
        name owner;
        uint32_t datetime;
        uint32_t wintype;
        uint32_t ratio;
        uint32_t bet;
        uint32_t betwin;
        uint32_t card1;
        uint32_t card2;
        uint32_t card3;
        uint32_t card4;
        uint32_t card5;

        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(st_events, (id)(owner)(datetime)(wintype)(ratio)(bet)(betwin)(card1)(card2)(card3)(card4)(card5))
    };

    // @abi table secrets i64
    struct st_secrets {
        uint64_t id;
        uint64_t s1;
        uint64_t primary_key() const { return id; }
        EOSLIB_SERIALIZE(st_secrets, (id)(s1))
    };

    typedef multi_index<N(metadatas), st_metadatas> _tb_metadatas;
    _tb_metadatas metadatas;
    typedef multi_index<N(pools), st_pools> _tb_pools;
    _tb_pools pools;
    typedef multi_index<N(events), st_events> _tb_events;
    _tb_events events;
    typedef multi_index<N(secrets), st_secrets> _tb_secrets;
    _tb_secrets secrets;
};