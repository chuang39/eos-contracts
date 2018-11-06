#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/time.hpp>

#include <string>

using namespace eosio;
using std::string;

class mev : public contract {
  public:
     mev( account_name self ):contract(self),bbregs(_self, _self),holders(_self, _self),rentrys(_self, _self){};
     //@abi action
     void create( account_name issuer,
                  asset        maximum_supply);
    //@abi action
     void issue( account_name to, asset quantity, string memo );
    //@abi action
     void transfer( account_name from,
                    account_name to,
                    asset        quantity,
                    string       memo );
    //@abi action
    void debug(account_name from);
    //@abi action
    void registerbb(account_name from, asset quantity);
    //@abi action
    void cancelreward(account_name from);
    //@abi action
    void addrentry(uint32_t month, uint32_t status);
    //@abi action
    void removerentry(uint32_t month);
    //@abi action
    void clear(const name from);
    //@abi action
    void test(const name from, string s);


    void rewardholders(const currency::transfer &t, account_name code);
    inline asset get_supply( symbol_name sym )const;

    inline asset get_balance( account_name owner, symbol_name sym )const;

  private:
     //@abi table accounts i64
     struct account {
        asset    balance;

        uint64_t primary_key()const { return balance.symbol.name(); }
     };

     //@abi table stat i64
     struct statt {
        asset          supply;
        asset          max_supply;
        account_name   issuer;

        uint64_t primary_key()const { return supply.symbol.name(); }
     };

     //@abi table rentrys i64
     struct st_rentrys {
         uint32_t startmonth;
         uint32_t expireat;
         uint32_t buybackstatus;
         uint64_t buybackshares;
         uint64_t buybackprice;
         uint64_t dividend;
         uint64_t dvddpershare;
         uint64_t profit;

         uint64_t primary_key()const { return startmonth; }
         EOSLIB_SERIALIZE(st_rentrys, (startmonth)(expireat)(buybackstatus)(buybackshares)(buybackprice)(dividend)(dvddpershare)(profit))
     };
     typedef multi_index<N(rentrys), st_rentrys> _tb_rentrys;
     _tb_rentrys rentrys;

     //@abi table bbregs i64
     struct st_bbregs {
         account_name owner;
         asset shares;

         uint64_t primary_key()const { return owner; }
         EOSLIB_SERIALIZE(st_bbregs, (owner)(shares))
     };
     typedef multi_index<N(bbregs), st_bbregs> _tb_bbregs;
     _tb_bbregs bbregs;

     struct st_holders {
         account_name owner;
         uint64_t primary_key()const { return owner; }
         EOSLIB_SERIALIZE(st_holders, (owner))
     };
     typedef multi_index<N(holders), st_holders> _tb_holders;
     _tb_holders holders;

     typedef multi_index<N(accounts), account> accounts;
     typedef multi_index<N(stat), statt> stats;

     void sub_balance( account_name owner, asset value );
     void add_balance( account_name owner, asset value, account_name ram_payer );


    //@abi table dividends i64
    struct st_dividends {
        account_name owner;
        uint64_t mevamount;
        uint64_t eosamount;

        uint64_t primary_key()const { return owner; }
        EOSLIB_SERIALIZE(st_dividends, (owner)(mevamount)(eosamount))
    };

    //@abi table buybacks i64
    struct st_buybacks {
        account_name owner;
        uint64_t mevamount;
        uint64_t eosamount;

        uint64_t primary_key()const { return owner; }
        EOSLIB_SERIALIZE(st_buybacks, (owner)(mevamount)(eosamount))
    };


  public:
     struct transfer_args {
        account_name  from;
        account_name  to;
        asset         quantity;
        string        memo;
     };

    struct st_stake_summary {
        name owner;
        asset stake;
        uint64_t primary_key() const { return owner; }
    };
    typedef multi_index<N(stakesummary), st_stake_summary> stake_summary_index;
};

asset mev::get_supply( symbol_name sym )const
{
  stats statstable( _self, sym );
  const auto& st = statstable.get( sym );
  return st.supply;
}

asset mev::get_balance( account_name owner, symbol_name sym )const
{
  accounts accountstable( _self, owner );
  const auto& ac = accountstable.get( sym );
  return ac.balance;
}

