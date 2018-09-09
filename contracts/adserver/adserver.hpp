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


class adserver : public eosio::contract {

public:
    adserver(account_name self)
            :contract(self){};
    //@abi action
    void getrequest(account_name from, uint64_t uuid);
};