//
// Created by Kevin Huang on 7/20/18.
//

#ifndef PROJECT_SCAM_H
#define PROJECT_SCAM_H
#endif //PROJECT_SCAM_H

/*
 * Dude, wrong file to check comments.
 */

//#include <math.h>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/time.hpp>

using namespace eosio;
using std::string;
using std::hash;


class scam : public eosio::contract {

public:
    scam(account_name self)
            :contract(self),
             pools(_self, _self),
             accounts(_self, _self),
             referrals(_self, _self),
             finaltable(_self, _self){};

    struct st_withdraw {
        name to;
    };

    //@abi action
    void createpool(const name owner, const string poolname, const uint32_t starttime);
    //@abi action
    void ping();
    //@abi action
    void pong();
    //@abi action
    void pong2(const name to);
    //@abi action
    void updatetime(uint32_t epoch);

    bool checkpool(uint32_t cur_in_sec);
    void deposit(const currency::transfer &t, account_name code);
    void runwithdraw(const st_withdraw &toaccount);

private:
    const static uint64_t DAY_IN_SEC = 3600 * 24;
    constexpr static uint64_t TIME_INC = 30;
    constexpr static uint64_t COOLING_TIME_IN_SEC = 3600 * 12;
    constexpr static double DIVIDEND_PERCENT = 0.20;
    constexpr static double REFERRAL_PERCENT = 0.05;
    constexpr static double FINAL_PRIZE_PERCENT = 0.17;
    constexpr static double FINAL_TABLE_PERCENT = 0.50;  // percentage of total prize
    constexpr static double FINAL_TABLE_PORTION = 0.15;
    constexpr static double LOYALTY_REWARDING = 1.01;
    //constexpr static double KEY_CARRYOVER = 0.1;
    constexpr static account_name TEAM_NAME = N(gyytmnbwgige);

    // @abi table pools i64
    struct st_pools {
        uint64_t id;
        string poolname;
        name owner;
        name lastbuyer;
        uint8_t status; // 0 for inactive; 1 for active
        uint32_t round;
        uint32_t created_at;
        uint32_t end_at;
        uint32_t last_buy_ts;
        uint64_t key_balance;
        uint64_t eos_balance;   // final prize
        uint64_t key_price;
        uint64_t eos_total;    // total incoming money
        uint64_t dividend_paid;
        uint64_t total_time_in_sec;
        string lastcomment;

        uint64_t primary_key() const { return id; }

        EOSLIB_SERIALIZE(st_pools, (id)(poolname)(owner)(lastbuyer)(status)(round)
                (created_at)(end_at)(last_buy_ts)(key_balance)(eos_balance)(key_price)(eos_total)
                (dividend_paid)(total_time_in_sec)
                (lastcomment))
    };
    typedef multi_index<N(pools), st_pools> _tb_pools;
    _tb_pools pools;

    // @abi table accounts i64
    struct st_accounts {
        name owner; // account name
        uint64_t key_balance;   // total keys
        uint64_t eos_balance;   // total balance: dividend + referral bonus
        uint64_t ref_balance;   // ref bonus
        uint64_t ft_balance;    // final table balance
        name referee;
        uint64_t finaltable_keys;

        uint64_t primary_key() const {return owner;}
        EOSLIB_SERIALIZE(st_accounts, (owner)(key_balance)(eos_balance)
                (ref_balance)(ft_balance)(referee)(finaltable_keys))
    };
    typedef multi_index<N(accounts), st_accounts> _tb_accounts;
    _tb_accounts accounts;

    // @abi table referrals i64
    struct st_referrals {
        uint64_t id;
        name owner;

        uint64_t get_referral_by_owner() const {return owner;}

        uint64_t primary_key() const {return id;}
        EOSLIB_SERIALIZE(st_referrals, (id)(owner))
    };
    typedef multi_index<N(referrals), st_referrals,
            indexed_by<N(byowner), const_mem_fun<st_referrals, uint64_t, &st_referrals::get_referral_by_owner>>
    > _tb_referrals;
    _tb_referrals referrals;

    // @abi table finaltable i64
    struct st_finaltable {
        name owner;
        uint64_t start;
        uint64_t end;

        uint64_t primary_key() const {return end;}
        EOSLIB_SERIALIZE(st_finaltable, (owner)(start)(end))
    };
    typedef multi_index<N(finaltable), st_finaltable> _tb_finaltable;
    _tb_finaltable finaltable;
};