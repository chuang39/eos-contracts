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
     mev( account_name self ):contract(self),buybackregs(_self, _self),holders(_self, _self),rewardinfos(_self, _self){};
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
    void addreward(uint32_t month, uint32_t status);
    //@abi action
    void removereward(uint32_t month);


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

     //@abi rewardinfos i64
     struct st_rewardinfos {
         uint32_t startmonth;
         uint32_t expireat;
         uint32_t buybackstatus;
         uint64_t buybackshares;
         uint64_t buybackprice;
         uint64_t dividend;
         uint64_t dvddpershare;
         uint64_t profit;

         uint64_t primary_key()const { return startmonth; }
         EOSLIB_SERIALIZE(st_rewardinfos, (startmonth)(buybackstatus)(buybackshares)(buybackprice)(dividend)(dvddpershare)(profit))
     };
     typedef multi_index<N(rewardinfos), st_rewardinfos> _tb_rewardinfos;
     _tb_rewardinfos rewardinfos;

     //@abi buybackregs i64
     struct st_buybackregs {
         account_name owner;
         asset shares;

         uint64_t primary_key()const { return owner; }
         EOSLIB_SERIALIZE(st_buybackregs, (owner)(shares))
     };
     typedef multi_index<N(buybackregs), st_buybackregs> _tb_buybackregs;
     _tb_buybackregs buybackregs;

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


  public:
     struct transfer_args {
        account_name  from;
        account_name  to;
        asset         quantity;
        string        memo;
     };
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

