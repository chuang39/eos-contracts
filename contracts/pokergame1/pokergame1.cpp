#include "pokergame1.hpp"

#define DEBUG 0

// (# of mev) = (# of eos) * miningtable[][1] / 100
uint64_t miningtable[5][2] = {{200000000000, 400}, // 1EOS 4MEV
                              {500000000000, 200}, // 1EOS 2MEV
                               {1000000000000, 100}, // 1EOS 1MEV
                               {1600000000000, 50}, // 2EOS 1MEV
                               {2000000000000, 25} // 4EOS 1MEV
                              };

uint64_t exptable[15] = {500000, 3000000, 9000000, 21000000, 61000000, 208500000, 654500000, 1783000000, 4279500000,
                         9269500000, 18490500000, 34500000000, 60922000000, 102736000000, 166608500000};

uint32_t bjvalues[13] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10};

// "IQ", "MEETONE", "BLACK", "BT", "EGT", "TPT", "ZKS", "KARMA"
//string tokentable[1] = {"MEV"};

// experience multiplier by game
uint32_t expbygame[5] = {50, 50, 12, 12, 50};
uint32_t minebygame[5] = {100, 100, 25, 25, 100};      // xN/100


// check if the player wins or not
uint32_t ratios[10] = {0, 1, 2, 3, 4, 5, 8, 25, 50, 250};
const uint32_t starttime = 1537833600; // 18/09/25 00:00:00 UTC
const uint32_t wstarttime = 1537747200; //GMT: Monday, September 24, 2018 12:00:00 AM
uint64_t mstarttimes[5] = {1535760000, 1538352000, 1541030400, 1543622400, 1546300800};
const char hex_chars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

uint32_t getlevel(uint64_t totalexp) {
    for (int i = 0; i < 15; i++) {
        if (totalexp < exptable[i]) {
            return i;
        }
    }
    return 15;
}

uint64_t getminingtableprice(uint64_t sold_keys) {
    for (int i = 0; i < 5; i++) {
        if (sold_keys < miningtable[i][0]) {
            uint32_t base = miningtable[i][1];
            return ((float)(miningtable[i][0] - sold_keys) / (float)(miningtable[i][0] - miningtable[i-1][0])) * (base - miningtable[i+1][1]) + miningtable[i+1][1];
        }
    }
    return 5;
}

uint32_t getstartweek(uint32_t epochtime) {
    return epochtime - ((epochtime - wstarttime) % 604800);
}

uint32_t getstartday(uint32_t epochtime) {
    return epochtime - ((epochtime - starttime) % 86400);
}

uint32_t getstartmonth(uint32_t epochtime) {
    for (int i = 0; i < 5; i++) {
        if (epochtime <= mstarttimes[i]) {
            return mstarttimes[i-1];
        }
    }
    return mstarttimes[4];
}

void sanity_check(uint64_t code, action_name act) {
    char buffer[32];

    for (int cnt = 0; cnt < 1; cnt++) {
        uint32_t na = get_action(1, cnt, buffer, 0);

        if (na == -1) return;
        eosio_assert(cnt == 0, "Invalid request!");

        action a = get_action(1, cnt);
        eosio_assert(a.account == code, "Invalid request!");
        eosio_assert(a.name == act, "Invalid request!");
        auto v = a.authorization;
        eosio_assert(v.size() == 1, "Invalid request!!");

        permission_level pl = v[0];
        eosio_assert(pl.permission == N(active) || pl.permission == N(owner), "Invalid request!!");
    }
}

checksum256 pokergame1::gethash(account_name from, uint32_t externalsrc, uint32_t rounds) {
    checksum256 result;
    return result;
/*
    auto itr_sec2 = secrets2.find(from);
    if (itr_sec2 == secrets2.end()) {
        uint32_t idx = from % 1024;

        auto itr_secret = secrets.find(idx);
        itr_sec2 = secrets2.emplace(_self, [&](auto &p){
            p.owner = name{from};
            p.s1 = itr_secret->s1;
        });
    }

    uint64_t seed = ((itr_sec2->s1 >> 16) | (itr_sec2->s1 << 48)) + from * 7 + externalsrc * 3;

    checksum256 result;
    sha256((char *)&seed, sizeof(seed), &result);

    uint64_t bias = 0;
    for (int i = 0; i < 8; i++) {
        bias |= result.hash[7 - i];
        bias <<= 8;
    }

    uint64_t src64 = ((uint64_t)externalsrc) << 2;
    //uint64_t round64 = ((uint64_t)rounds) << 32;
    bias = bias + src64;

    secrets2.modify(itr_sec2, _self, [&](auto &p){
        p.s1 = bias;
    });

    return result;
    */
    /*
    auto itr_metadata = metadatas.find(0);
    auto itr_secret = secrets.find(itr_metadata->idx);
    uint64_t seed = itr_secret->s1 + from * 7 + externalsrc * 3;

    checksum256 result;
    sha256((char *)&seed, sizeof(seed), &result);

    uint64_t bias = 0;
    for (int i = 0; i < 8; i++) {
        bias |= result.hash[7 - i];
        bias <<= 8;
    }

    uint64_t src64 = ((uint64_t)externalsrc) << 2;
    uint64_t round64 = ((uint64_t)rounds) << 32;
    bias = bias + src64 + round64;

    // TODO: do we need to sync with chain?
    //if (rounds % 4096 == 0) {
    //    bias += current_time() + tapos_block_num();
    //}

    uint64_t lastidx = itr_metadata->idx == 0 ? 1023 : itr_metadata->idx - 1;
    auto itr_last = secrets.find(lastidx);
    secrets.modify(itr_last, _self, [&](auto &p){
        p.s1 = bias;
    });
    metadatas.modify(itr_metadata, _self, [&](auto &p){
        p.idx = (p.idx + 1) % 1024;
    });

    return result;
     */
}

void pokergame1::getcards(account_name from, checksum256 result, uint32_t* cards, uint32_t length, std::set<uint32_t> myset, uint64_t hack, uint32_t maxcard) {
    uint32_t cnt = 0;
    uint32_t pos = 0;
    checksum256 lasthash = result;

    while (cnt < length) {
        uint64_t ctemp = lasthash.hash[pos];
        ctemp <<= 32;
        ctemp |= lasthash.hash[pos+1];
        uint32_t card = (uint32_t)(ctemp % maxcard);
        if (myset.find(card) != myset.end()) {
            pos++;
            if (pos == 32) {
                pos = 0;
                checksum256 newhash;
                sha256((char *)&lasthash.hash, 32, &newhash);
                lasthash = newhash;
            }
            continue;
        }
        myset.insert(card);
        cards[cnt] = card;
        //print("====Get Card:", card);

        // update the drawn card in cardstats
        auto itr_cardstat = cardstats.find(card % 52);
        eosio_assert(itr_cardstat != cardstats.end(), "Cannot find the card in card statistic table.");
        cardstats.modify(itr_cardstat, _self, [&](auto &p) {
            p.count += 1;
        });

        cnt++;
        pos++;
        if (pos == 32) {
            pos = 0;
            checksum256 newhash;
            sha256((char *)&lasthash.hash, 32, &newhash);
            lasthash = newhash;
        }
    }
}

void pokergame1::signup(const name from, const string memo) {
    require_auth(from);
/*
    auto itr_user1 = suaccounts.find(from);
    eosio_assert(itr_user1 == suaccounts.end(), "User is already signed up.");
    asset bal2 = asset(1000000, symbol_type(S(4, MEV)));

    // report mining
    uint64_t minemev = 1000000;
    uint64_t meosin = 0;
    uint64_t meosout = 0;
    report(from, minemev, meosin, meosout);

    action(permission_level{_self, N(active)}, N(eosvegascoin),
           N(transfer), std::make_tuple(N(eosvegasjack), from, bal2,
                                        std::string(name{from}.to_string() + ", welcome on board! 100 MEV is prepared as gift for your journey!")))
            .send();

    itr_user1 = suaccounts.emplace(_self, [&](auto &p){
        p.owner = name{from};
    });
    */
}


uint32_t get_bj_num(uint32_t card) {
    return card % 13;
}

uint32_t get_bj_color(uint32_t card) {
    return (card % 52) / 13;
}



/********************* Blackjack *****************************/

uint32_t pokergame1::bj_get_stat(uint32_t status) {
    return status & 0xff;
}

void pokergame1::bj_get_cards(uint64_t cards, uint32_t count, uint32_t* arr) {
    for (int i = 0; i < count; i++) {
        arr[i] = ((cards >> (8 * i)) & 0xff);
    }
}

uint32_t bj_get_result(uint32_t* arr, uint32_t count) {
    uint32_t aces = 0;
    uint32_t value = 0;

    for (int i = 0; i < count; i++) {
        uint32_t num = arr[i] % 13;
        if (num == 0) aces++;
        value += bjvalues[num];
    }

    // bust
    if (value > 21) return value;

    for (int i = 0; i < aces; i++) {
        if (value + 10 > 21) {
            break;
        }
        value += 10;
    }

    return value;
}

bool isBlackjack(uint32_t c1, uint32_t c2) {
    uint32_t n1 = c1 % 13;
    uint32_t n2 = c2 % 13;


    if (n1 == 0 && n2 >=9) return true;
    if (n2 == 0 && n1 >=9) return true;

    return false;
}

void pokergame1::bjuninsure(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
        std::vector<uint32_t> player_hand1, std::vector<uint32_t> player_hand2) {
    require_recipient(player);

    auto itr_bjpool = bjpools.find(player);
    eosio_assert(itr_bjpool != bjpools.end(), "Blackjack: user pool not found");
    eosio_assert(itr_bjpool->nonce == nonce, "Blackjack: nonce does not match");

    uint32_t standcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'S');
    eosio_assert(standcnt == 0, "Blackjack: action uninsure cannot perform after stand");

    uint32_t hitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'H');
    eosio_assert(hitcnt == 0, "Blackjack: action uninsure cannot perform after hit");

    uint32_t uninsurecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'U');
    eosio_assert(uninsurecnt == 0, "Blackjack: action uninsure cannot perform twice");

    uint32_t insurecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'I');
    eosio_assert(insurecnt == 0, "Blackjack: action uninsure cannot perform after insuring");

    bjpools.modify(itr_bjpool, _self, [&](auto &p){
        p.actions = p.actions + "U";
    });

    // clear bjmevouts;
    auto itr_bjmevout = bjmevouts.begin();
    uint32_t cnt = 0;
    while (itr_bjmevout != bjmevouts.end() && cnt < 16) {
        if (itr_bjmevout->lastseen + 3600 < now()) {
            itr_bjmevout = bjmevouts.erase(itr_bjmevout);
        } else {
            itr_bjmevout++;
        }
        cnt++;
    }
}

void pokergame1::bjhit(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
                       std::vector<uint32_t> player_hand1, std::vector<uint32_t> player_hand2) {
    require_recipient(player);

    auto itr_bjpool = bjpools.find(player);
    eosio_assert(itr_bjpool != bjpools.end(), "Blackjack: user pool not found");
    eosio_assert(itr_bjpool->nonce == nonce, "Blackjack: nonce does not match");

    // if dealer first card is ace, we should see either uninsurance or insurance first
    if (dealer_hand[0] % 13 == 0) {
        uint32_t insurecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'I');
        uint32_t uninsurecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'U');
        eosio_assert(insurecnt == 1 || uninsurecnt == 1, "Blackjack: must insure or uninsure first");
    }

    uint32_t player1_cnt = player_hand1.size();
    uint32_t hcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'H');
    eosio_assert((hcnt + 2) == player1_cnt, "Blackjack: cards don't match for the hit");
    uint32_t standcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'S');
    eosio_assert(standcnt == 0, "Blackjack: action hit cannot perform after stand");
    uint32_t splitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Y');
    eosio_assert(splitcnt == 0, "Blackjack: action stand cannot perform after split");

    bjpools.modify(itr_bjpool, _self, [&](auto &p){
        p.actions = p.actions + "H";
    });
}

void pokergame1::bjstand(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
        std::vector<uint32_t> player_hand1, std::vector<uint32_t> player_hand2) {
    require_recipient(player);

    auto itr_bjpool = bjpools.find(player);
    eosio_assert(itr_bjpool != bjpools.end(), "Blackjack: user pool not found");
    eosio_assert(itr_bjpool->nonce == nonce, "Blackjack: nonce does not match");

    // if dealer first card is ace, we should see either uninsurance or insurance first
    if (dealer_hand[0] % 13 == 0) {
        uint32_t insurecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'I');
        uint32_t uninsurecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'U');
        eosio_assert(insurecnt == 1 || uninsurecnt == 1, "Blackjack: must insure or uninsure first");
    }

    uint32_t standcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'S');
    eosio_assert(standcnt == 0, "Blackjack: action stand has executed");
    uint32_t doublecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'D');
    eosio_assert(doublecnt == 0, "Blackjack: action stand cannot perform after double");
    uint32_t splitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Y');
    eosio_assert(splitcnt == 0, "Blackjack: action stand cannot perform after split");

    uint32_t player1_cnt = player_hand1.size();
    uint32_t hcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'H');
    eosio_assert((hcnt + 2) == player1_cnt, "Blackjack: cards don't match for the hit");

    bjpools.modify(itr_bjpool, _self, [&](auto &p){
        p.actions = p.actions + "S";
    });
}

// hand1: hit-Q, stand-W; hand2: hit-E, stand-R
void pokergame1::bjhit1(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
                       std::vector<uint32_t> player_hand1) {
    require_recipient(player);

    auto itr_bjpool = bjpools.find(player);
    eosio_assert(itr_bjpool != bjpools.end(), "Blackjack: user pool not found");
    eosio_assert(itr_bjpool->nonce == nonce, "Blackjack: nonce does not match");

    uint32_t player1_cnt = player_hand1.size();
    uint32_t hcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Q');
    eosio_assert((hcnt + 2) == player1_cnt, "Blackjack: cards don't match for the hit1");

    uint32_t splitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Y');
    eosio_assert(splitcnt == 1, "Blackjack: action hit1 must follow split");
    uint32_t hit2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'E');
    eosio_assert(hit2cnt == 0, "Blackjack: action hit1 cannot perform after hit2");
    uint32_t stand2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'R');
    eosio_assert(stand2cnt == 0, "Blackjack: action hit1 cannot perform after stand2");
    uint32_t stand1cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'W');
    eosio_assert(stand1cnt == 0, "Blackjack: action hit1 cannot perform after stand1");

    if (itr_bjpool->pcards1 != 0 && itr_bjpool->pcards2 != 0) {
        eosio_assert(itr_bjpool->pcards1 % 13 != 0 || itr_bjpool->pcards2 % 13 != 0,
                     "Blackjack: cannot hit1 after split aces");
    }

    bjpools.modify(itr_bjpool, _self, [&](auto &p){
        p.actions = p.actions + "Q";
    });
}

void pokergame1::bjstand1(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
                         std::vector<uint32_t> player_hand1) {
    require_recipient(player);

    auto itr_bjpool = bjpools.find(player);
    eosio_assert(itr_bjpool != bjpools.end(), "Blackjack: user pool not found");
    eosio_assert(itr_bjpool->nonce == nonce, "Blackjack: nonce does not match");

    uint32_t splitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Y');
    eosio_assert(splitcnt == 1, "Blackjack: action stand1 must follow split");

    uint32_t player1_cnt = player_hand1.size();
    uint32_t hcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Q');
    eosio_assert((hcnt + 2) == player1_cnt, "Blackjack: cards don't match for the hit");
    uint32_t hit2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'E');
    eosio_assert(hit2cnt == 0, "Blackjack: action stand1 cannot perform after hit2");
    uint32_t stand2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'R');
    eosio_assert(stand2cnt == 0, "Blackjack: action stand1 cannot perform after stand2");
    uint32_t stand1cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'W');
    eosio_assert(stand1cnt == 0, "Blackjack: action stand1 cannot perform twice");

    bjpools.modify(itr_bjpool, _self, [&](auto &p){
        p.actions = p.actions + "W";
    });
}

void pokergame1::bjhit2(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
                        std::vector<uint32_t> player_hand2) {
    require_recipient(player);

    auto itr_bjpool = bjpools.find(player);
    eosio_assert(itr_bjpool != bjpools.end(), "Blackjack: user pool not found");
    eosio_assert(itr_bjpool->nonce == nonce, "Blackjack: nonce does not match");

    uint32_t player2_cnt = player_hand2.size();
    uint32_t hcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'E');
    eosio_assert((hcnt + 2) == player2_cnt, "Blackjack: cards don't match for the hit2");

    uint32_t splitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Y');
    eosio_assert(splitcnt == 1, "Blackjack: action hit1 must follow split");
    uint32_t stand2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'R');
    eosio_assert(stand2cnt == 0, "Blackjack: action hit1 cannot perform after stand2");

    if (itr_bjpool->pcards1 != 0 && itr_bjpool->pcards2 != 0) {
        eosio_assert(itr_bjpool->pcards1 % 13 != 0 || itr_bjpool->pcards2 % 13 != 0,
                     "Blackjack: cannot hit2 after split aces");
    }

    bjpools.modify(itr_bjpool, _self, [&](auto &p){
        p.actions = p.actions + "E";
    });
}

void pokergame1::bjstand2(const name player, uint32_t nonce, std::vector<uint32_t> dealer_hand,
                          std::vector<uint32_t> player_hand2) {
    require_recipient(player);

    auto itr_bjpool = bjpools.find(player);
    eosio_assert(itr_bjpool != bjpools.end(), "Blackjack: user pool not found");
    eosio_assert(itr_bjpool->nonce == nonce, "Blackjack: nonce does not match");

    uint32_t splitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Y');
    eosio_assert(splitcnt == 1, "Blackjack: action stand2 must follow split");

    uint32_t player2_cnt = player_hand2.size();
    uint32_t hcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'E');
    eosio_assert((hcnt + 2) == player2_cnt, "Blackjack: cards don't match for the hit");
    uint32_t stand2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'R');
    eosio_assert(stand2cnt == 0, "Blackjack: action stand2 cannot perform twice");

    bjpools.modify(itr_bjpool, _self, [&](auto &p){
        p.actions = p.actions + "R";
    });
}

void pokergame1::bjreceipt(string game_id, const name player, string game, string seed,  std::vector<string> dealer_hand,
                            std::vector<string> player_hand1, std::vector<string> player_hand2, string bet, string win,
                            string insure_bet, string insure_win, uint64_t betnum, uint64_t winnum, uint64_t insurance,
                            uint64_t insurance_win, string token, string actions, string pub_key) {

    require_auth(N(eosvegasjack));
    require_recipient(player);

    auto itr_bjpool = bjpools.find(player);
    auto itr_metadata = metadatas.find(0);

    eosio_assert(itr_bjpool != bjpools.end(), "Blackjack: user pool not found");
    eosio_assert(itr_bjpool->bet > 0, "Blackjack:bet must be larger than zero");

    int pos1 = seed.find("_");
    eosio_assert(pos1 > 0, "Blackjack: seed is incorrect.");
    string ucm = seed.substr(0, pos1);
    uint32_t seednonce = stoi(ucm);
    eosio_assert(seednonce == itr_bjpool->nonce, "Blackjack: nonce does not match.");

    eosio_assert(betnum == itr_bjpool->bet, "Blackjack: bet does not match.");
    eosio_assert(token == itr_bjpool->bettoken, "Blackjack: bet token does not match.");

    if (token == "EOS") {
        auto itr_paccount = paccounts.find(player);
        eosio_assert(itr_paccount != paccounts.end(), "Blackjack: user not found");

        asset bal = asset((winnum + insurance_win), symbol_type(S(4, EOS)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosio.token),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- blackjack.rovegas.com")))
                    .send();
        }

        uint64_t mineprice = getminingtableprice(itr_metadata->tmevout);
        uint64_t minemev = (betnum + insurance) * mineprice * (1 + itr_paccount->level * 0.05) * minebygame[2] / (100*100);

        report(player, minemev, (betnum + insurance), (winnum + insurance_win));
        updatemevout(player, itr_bjpool->nonce, minemev, 2);

        asset bal2 = asset(minemev, symbol_type(S(4, MEV)));
        if (bal2.amount > 0) {
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(N(eosvegasjack), player, bal2,
                                                std::string("Gaming deserves rewards! - blackjack.rovegas.com")))
                    .send();
        }
    } else if (token == "MEV") {
        asset bal = asset((winnum + insurance_win), symbol_type(S(4, MEV)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- blackjack.rovegas.com")))
                    .send();
        }
    }

    bjpools.erase(itr_bjpool);
}

void pokergame1::payref(name from, uint64_t bet, uint32_t defaultrate, uint32_t multiplier) {
    if (bet == 0) return;

    bool isPartnerPaid = false;
    auto itr_pref = prefs.find(from);
    if (itr_pref != prefs.end()) {
        auto itr_partner = partners.find(itr_pref->partner);
        if (itr_partner != partners.end()) {
            uint64_t ratetemp = itr_partner->rate;
            uint64_t pay = (bet * ratetemp * multiplier) / 10000;

            prefs.erase(itr_pref);

            if (pay > 0) {
                asset bal = asset(pay, symbol_type(S(4, EOS)));
                action(permission_level{_self, N(active)}, N(eosio.token),
                       N(transfer), std::make_tuple(_self, itr_pref->partner, bal,
                                                    std::string(
                                                            "Growth is never by mere chance; it is the result of forces partnering together! We really appreciate our partnership! - Rovegas.com")))
                        .send();
                partners.modify(itr_partner, _self, [&](auto &p) {
                    p.balance += pay;
                    p.count += 1;
                });
                isPartnerPaid = true;
            }
        }
    }

    // pay referrals
    auto itr_ref = referrals.find(from);
    if (itr_ref == referrals.end()) return;
    if (from == N(eosvegasjack)) {
        referrals.erase(itr_ref);
        return;
    }

    uint64_t rate = defaultrate;
    auto itr_partner = partners.find(itr_ref->referrer);
    if (itr_partner != partners.end()) {
        rate = itr_partner->rate;
    }
    uint64_t pay = (bet * rate * multiplier) / 10000;
    if (pay == 0) {
        referrals.erase(itr_ref);
        return;
    }

    if (itr_partner == partners.end()) {
        asset bal = asset(pay, symbol_type(S(4, EOS)));
        action(permission_level{_self, N(active)}, N(eosio.token),
               N(transfer), std::make_tuple(_self, itr_ref->referrer, bal,
                                            std::string("Thanks for your referral. People love us on EOS! - ROVegas.com")))
                .send();
    } else {
        if (isPartnerPaid == false) {
            asset bal = asset(pay, symbol_type(S(4, EOS)));
            action(permission_level{_self, N(active)}, N(eosio.token),
                   N(transfer), std::make_tuple(_self, itr_ref->referrer, bal,
                                                std::string(
                                                        "Thanks for your referral. People love us on EOS! - Rovegas.com")))
                    .send();
            partners.modify(itr_partner, _self, [&](auto &p) {
                p.balance += pay;
                p.count += 1;
            });
        }
    }

    auto itr_refouts = refouts.find(0);
    if (itr_refouts == refouts.end()) {
        refouts.emplace(_self, [&](auto &p){
            p.eosout = pay;
            p.count = 1;
        });
    } else {
        refouts.modify(itr_refouts, _self, [&](auto &p) {
            p.eosout += pay;
            p.count += 1;
        });
    }

    referrals.erase(itr_ref);
}

uint32_t pokergame1::increment_nonce(const name user) {
    // Get current nonce and increment it
    uint32_t nonce =  0;
    auto itr_nonce = nonces.find(user);
    if (itr_nonce != nonces.end()) {
        nonce = itr_nonce->number;
    }
    if (itr_nonce == nonces.end()) {
        nonces.emplace(_self, [&](auto &p) {
            p.owner = name{user};
            p.number = 1;
        });
    } else {
        nonces.modify(itr_nonce, _self, [&](auto &p) {
            p.number += 1;
        });
    }
    return nonce;
}

uint32_t pokergame1::increment_bjnonce(const name user) {
    // Get current nonce and increment it
    uint32_t bjnonce =  0;
    auto itr_bjnonce = bjnonces.find(user);
    if (itr_bjnonce != bjnonces.end()) {
        bjnonce = itr_bjnonce->number;
    }
    if (itr_bjnonce == bjnonces.end()) {
        bjnonces.emplace(_self, [&](auto &p) {
            p.owner = name{user};
            p.number = 1;
        });
    } else {
        bjnonces.modify(itr_bjnonce, _self, [&](auto &p) {
            p.number += 1;
        });
    }
    return bjnonce;
}

uint32_t pokergame1::increment_uthnonce(const name user) {
    // Get current nonce and increment it
    uint32_t uthnonce =  0;
    auto itr_uthnonce = uthnonces.find(user);
    if (itr_uthnonce != uthnonces.end()) {
        uthnonce = itr_uthnonce->number;
    }
    if (itr_uthnonce == uthnonces.end()) {
        uthnonces.emplace(_self, [&](auto &p) {
            p.owner = name{user};
            p.number = 1;
        });
    } else {
        uthnonces.modify(itr_uthnonce, _self, [&](auto &p) {
            p.number += 1;
        });
    }
    return uthnonce;
}

uint32_t pokergame1::increment_pdnonce(const name user) {
    // Get current nonce and increment it
    uint32_t pdnonce =  0;
    auto itr_pdnonce = pdnonces.find(user);
    if (itr_pdnonce != pdnonces.end()) {
        pdnonce = itr_pdnonce->number;
    }
    if (itr_pdnonce == pdnonces.end()) {
        pdnonces.emplace(_self, [&](auto &p) {
            p.owner = name{user};
            p.number = 1;
        });
    } else {
        pdnonces.modify(itr_pdnonce, _self, [&](auto &p) {
            p.number += 1;
        });
    }
    return pdnonce;
}

void pokergame1::deposit(const currency::transfer &t, account_name code, uint32_t bettype) {
    // run sanity check here
    if (code == _self) {
        return;
    }

    auto itr_blacklist = blacklists.find(t.from);
    eosio_assert(itr_blacklist == blacklists.end(), "Sorry, please be patient.");

    // No bet from eosvegascoin
    if (t.from == N(eosvegascoin) || t.from == N(eosvegascorp) || t.from == N(eosvegasopmk) || t.from == N(gyytmnbwgige)) {
        return;
    }

    bool iseos = bettype == 0 ? true : false;
    string bettoken = "EOS";
    if (bettype == 0) {
        eosio_assert(code == N(eosio.token), "EOS should be sent by eosio.token");
        eosio_assert(t.quantity.symbol == string_to_symbol(4, "EOS"), "Incorrect token type.");
        sanity_check(N(eosio.token), N(transfer));
    } else if (bettype == 1) {
        eosio_assert(code == N(eosvegascoin), "MEV should be sent by eosvegascoin.");
        eosio_assert(t.quantity.symbol == string_to_symbol(4, "MEV"), "Incorrect token type.");
        bettoken = "MEV";
        sanity_check(N(eosvegascoin), N(transfer));
    } else if (bettype == 2) {
        eosio_assert(code == N(horustokenio), "HORUS should be sent by horustokenio.");
        eosio_assert(t.quantity.symbol == string_to_symbol(4, "HORUS"), "Incorrect token type.");
        bettoken = "HORUS";
        sanity_check(N(horustokenio), N(transfer));
    } else if (bettype == 3) {
        eosio_assert(code == N(everipediaiq), "IQ should be sent by everipediaiq.");
        eosio_assert(t.quantity.symbol == string_to_symbol(4, "IQ"), "Incorrect token type.");
        bettoken = "IQ";
        sanity_check(N(everipediaiq), N(transfer));
    } else if (bettype == 4) {
        eosio_assert(code == N(bitpietokens), "EUSD should be sent by bitpietokens.");
        eosio_assert(t.quantity.symbol == string_to_symbol(8, "EUSD"), "Incorrect token type.");
        bettoken = "EUSD";
        sanity_check(N(bitpietokens), N(transfer));
    }

    eosio_assert(t.to == _self, "Transfer not made to this contract");
    eosio_assert(t.quantity.is_valid(), "Invalid token transfer");
    eosio_assert(t.quantity.amount > 0, "Quantity must be positive");

    string usercomment = t.memo;
    uint32_t gameid = 0;    // 0: jacks-or-better; 1: jacks-or-better 5x
    if (usercomment.find("type[") == 0) {
        uint32_t pos = usercomment.find("]");
        if (pos > 0 && pos != 4294967295) {
            string ucm = usercomment.substr(5, pos - 5);
            gameid = stoi(ucm);
        }
    }

    string userseed = "";
    uint32_t seedidx = usercomment.find("seed[");
    if (seedidx > 0 && seedidx != 4294967295) {
        uint32_t pos = usercomment.find("]", seedidx);
        if (pos > 0 && pos != 4294967295) {
            userseed = usercomment.substr(seedidx + 5, pos - seedidx - 5);
        }
    }

    // detect referral
    uint32_t refidx = usercomment.find("ref[");
    if (refidx > 0 && refidx != 4294967295) {
        uint32_t pos = usercomment.find("]", refidx);
        if (pos > 0 && pos != 4294967295) {
            string ucm = usercomment.substr(refidx + 4, pos - refidx - 4);

            char * tab2 = new char [ucm.length()+1];
            strcpy (tab2, ucm.c_str());
            name referrer = name{string_to_name(tab2)};
            eosio_assert(referrer != t.from, "Sorry, you cannot refer yourself.");

            if ( is_account( referrer )) {
                auto itr_ref = referrals.find(t.from);
                if (itr_ref == referrals.end()) {
                    referrals.emplace(_self, [&](auto &p){
                        p.owner = name{t.from};
                        p.referrer = referrer;
                    });
                } else {
                    referrals.modify(itr_ref, _self, [&](auto &p) {
                        p.referrer = referrer;
                    });
                }
            }
        }
    }

    // detect partner
    uint32_t partneridx = usercomment.find("partner[");
    if (partneridx > 0 && partneridx != 4294967295) {
        uint32_t pos = usercomment.find("]", partneridx);
        if (pos > 0 && pos != 4294967295) {
            string ucm = usercomment.substr(partneridx + 8, pos - partneridx - 8);

            char * tab2 = new char [ucm.length()+1];
            strcpy (tab2, ucm.c_str());
            name pref = name{string_to_name(tab2)};
            if ( is_account( pref )) {
                auto itr_ref = prefs.find(t.from);
                if (itr_ref == prefs.end()) {
                    prefs.emplace(_self, [&](auto &p){
                        p.owner = name{t.from};
                        p.partner = pref;
                    });
                } else {
                    prefs.modify(itr_ref, _self, [&](auto &p) {
                        p.partner = pref;
                    });
                }
            }
        }
    }

    // gameid 0: jacks-or-better
    // gameid 1: jacks-or-better 5x
    // gameid 2: blackjack
    // gameid 3: ultimate texas holdem
    // gameid 4: pokerdice
    eosio_assert((gameid == 0 || gameid == 1 || gameid == 2 || gameid == 3 || gameid == 4 || gameid == 88), "Non-recognized game id");
    // metadatas[1] is blackjack, metadatas[2] is video poker.
    auto itr_metadata = gameid == 2 ? metadatas.find(1) : metadatas.find(2);
    auto itr_metadata2 = metadatas.find(0);
    eosio_assert(itr_metadata != metadatas.end(), "Game is not found.");
    eosio_assert(itr_metadata2 != metadatas.end(), "No game is found.");

    eosio_assert(itr_metadata2->gameon == 1  || t.from == N(blockfishbgp), "All games are temporarily paused.");
    eosio_assert(itr_metadata->gameon == 1 || t.from == N(blockfishbgp), "Game is temporarily paused.");

    account_name user = t.from;

    //eosio_assert(userseed == vppools.end(), "Jacks-or-Better: user seed is empty!");
    if (gameid == 0 || gameid == 1) {
        eosio_assert(userseed.length() > 0, "user seed cannot by empty.");
        uint32_t nonce = increment_nonce(name{user});

        if (gameid == 0 && bettoken == "EOS") {
            eosio_assert(t.quantity.amount >= 1000, "Jacks-or-Better: Below minimum bet threshold!");
            eosio_assert(t.quantity.amount <= 2000000, "Jacks-or-Better:Exceeds bet cap!");
        } else if (gameid == 1 && bettoken == "EOS") {
            eosio_assert(t.quantity.amount >= 5000, "Jacks-or-Better 5x:Below minimum bet threshold!");
            eosio_assert(t.quantity.amount <= 10000000, "Jacks-or-Better 5x:Exceeds bet cap!");
        } else if (gameid == 0 && bettoken == "EUSD") {
            eosio_assert(t.quantity.amount >= 2000, "Jacks-or-Better: Below minimum bet threshold!");
            eosio_assert(t.quantity.amount <= 400000, "Jacks-or-Better:Exceeds bet cap!");
        } else if (gameid == 1 && bettoken == "EUSD") {
            eosio_assert(t.quantity.amount >= 10000, "Jacks-or-Better 5x:Below minimum bet threshold!");
            eosio_assert(t.quantity.amount <= 20000000, "Jacks-or-Better 5x:Exceeds bet cap!");
        }

        auto itr_vppool = vppools.find(user);

        eosio_assert(itr_vppool == vppools.end(), "Jacks-or-Better: your last round is not finished. Please contact admin!");

        uint32_t mode = (gameid == 0) ? 0 : 1;

        vppools.emplace(_self, [&](auto &p) {
            p.owner = name{user};
            p.status = 1;
            p.mode = mode;
            p.nonce = nonce;
            p.seed = userseed;
            p.bettoken = bettoken;
            p.bet = t.quantity.amount;
        });

        if (bettoken == "EOS") {
            /*
            // jackpot
            auto itr_pevent = pevents.find(0);
            pevents.modify(itr_pevent, _self, [&](auto &p) {
                uint64_t tempam = t.quantity.amount * 0.001;
                p.eosin += tempam;
            });
            */

            // new year
            auto itr_newyear = newyears.find(user);
            if (itr_newyear == newyears.end()) {
                newyears.emplace(_self, [&](auto &p) {
                    p.owner = name{user};
                    p.status = 0;
                    p.eosin = t.quantity.amount;
                });
            } else {
                newyears.modify(itr_newyear, _self, [&](auto &p) {
                    p.eosin += t.quantity.amount;
                });
            }

            auto itr_tnewyear = tnewyears.find(0);
            if (itr_tnewyear == tnewyears.end()) {
                tnewyears.emplace(_self, [&](auto &p) {
                    p.id = 0;
                    p.eosin = t.quantity.amount;
                });
            } else {
                tnewyears.modify(itr_tnewyear, _self, [&](auto &p) {
                    p.eosin += t.quantity.amount;
                });
            }

            payref(name{user}, t.quantity.amount, 1, 4);
        }
    } else if (gameid == 2) {
        uint32_t actionid = 0;
        uint32_t actionidx = usercomment.find("action[");
        if (actionidx > 0 && actionidx != 4294967295) {
            uint32_t actionpos = usercomment.find("]", actionidx);
            if (actionpos > 0 && actionpos != 4294967295) {
                string ucmaction = usercomment.substr(actionidx + 7, actionpos - actionidx - 7);
                actionid = stoi(ucmaction);
            }
        }

        eosio_assert(actionid == 1 || actionid == 2 || actionid == 3 || actionid == 4 || actionid == 5 || actionid == 6, "Blackjack: invalid action id.");
        uint32_t bjstatus = 0;
        auto itr_bjpool = bjpools.find(user);
        if (itr_bjpool == bjpools.end()) {
            eosio_assert(userseed.length() > 0, "user seed cannot by empty.");
            eosio_assert(actionid == 1, "Blackjack: deposit first");

            //cap
            if (bettoken == "EOS") {
                eosio_assert(t.quantity.amount >= 1000, "Blackjack: Below minimum bet threshold!");
                eosio_assert(t.quantity.amount <= 5000000, "Blackjack:Exceeds bet cap!");
            }

            uint32_t nonce = increment_bjnonce(name{user});

            if (itr_bjpool == bjpools.end()) {
                bjpools.emplace(_self, [&](auto &p) {
                    p.owner = name{user};
                    p.status = bjstatus;
                    p.nonce = nonce;
                    p.seed = userseed;
                    p.bettoken = bettoken;
                    p.bet = t.quantity.amount;
                });
            } else {
                bjpools.modify(itr_bjpool, _self, [&](auto &p) {
                    p.owner = name{user};
                    p.status = bjstatus;
                    p.nonce = nonce;
                    p.seed = userseed;
                    p.bettoken = bettoken;
                    p.bet = t.quantity.amount;
                });
            }

            payref(name{user}, t.quantity.amount, 1, 1);  // pay ref for deal
        } else if (gameid == 2) {
            eosio_assert(actionid != 1, "Blackjack: existing round is not finished");

            uint32_t gamenonce = 0;
            uint32_t nonceidx = usercomment.find("nonce[");
            if (nonceidx > 0 && nonceidx != 4294967295) {
                uint32_t noncepos = usercomment.find("]", nonceidx);
                if (noncepos > 0 && noncepos != 4294967295) {
                    string ucmaction = usercomment.substr(nonceidx + 6, noncepos - nonceidx - 6);
                    gamenonce = stoi(ucmaction);
                }
            }
            eosio_assert(itr_bjpool->nonce == gamenonce, "Blackjack: nonce does not match");

            if (actionid == 2) {
                bjstatus = 2;   // insurance status

                eosio_assert(itr_bjpool->actions.length() == 0, "Blackjack: no action should be performed before action insure");

                eosio_assert((itr_bjpool->bet / 2) == t.quantity.amount, "Blackjack: insurance must be half of wager.");
                bjpools.modify(itr_bjpool, _self, [&](auto &p){
                    p.actions = p.actions + "I";
                    p.insurance = t.quantity.amount;
                });
            } else if (actionid == 3) {
                bjstatus = 1;   // double so that status is done.

                uint32_t hitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'H');
                eosio_assert(hitcnt == 0, "Blackjack: action double cannot perform after hit");
                uint32_t doublecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'D');
                eosio_assert(doublecnt == 0, "Blackjack: action double cannot perform twice");
                uint32_t standcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'S');
                eosio_assert(standcnt == 0, "Blackjack: action double cannot perform after stand");
                uint32_t splitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Y');
                eosio_assert(splitcnt == 0, "Blackjack: action stand cannot perform after split");

                eosio_assert(itr_bjpool->bet == t.quantity.amount, "Blackjack: double must deposit the same amount as wager");
                bjpools.modify(itr_bjpool, _self, [&](auto &p){
                    p.actions = p.actions + "D";
                    p.bet = (p.bet * 2);
                });
            } else if (actionid == 4) {
                bjstatus = 10;   // split

                uint32_t hitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'H');
                eosio_assert(hitcnt == 0, "Blackjack: action split cannot perform after hit");
                uint32_t doublecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'D');
                eosio_assert(doublecnt == 0, "Blackjack: action split cannot perform twice");
                uint32_t standcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'S');
                eosio_assert(standcnt == 0, "Blackjack: action split cannot perform after stand");

                eosio_assert(itr_bjpool->bet == t.quantity.amount, "Blackjack: split must deposit the same amount as wager");

                /*
                int splitidx = usercomment.find("cards[");
                eosio_assert(splitidx >= 0, "Blackjack: cannot find cards in memo for split");

                int splitpos = usercomment.find("]", splitidx);
                eosio_assert(splitpos > 0, "Blackjack: cannot find cards in memo for split");
                uint32_t commapos = usercomment.find(",", splitidx);
                eosio_assert(commapos < splitpos, "Blackjack: invalid format in cards");

                string card1str = usercomment.substr(splitidx + 6, commapos - splitidx - 6);
                string card2str = usercomment.substr(commapos + 1, splitpos - commapos - 1);
                eosio_assert( stoi(card1str) % 13 == stoi(card2str) % 13, "Blackjack: two cards must be idential");
                eosio_assert( stoi(card1str) < 208, "Blackjack: card should be less than 208");
                */

                bjpools.modify(itr_bjpool, _self, [&](auto &p){
                    p.actions = p.actions + "Y";
                    p.bet = (p.bet * 2);
                    //p.pcards1 = stoi(card1str);
                    //p.pcards2 = stoi(card2str);
                });
            } else if (actionid == 5) {
                bjstatus = 11;   // double hand1

                uint32_t hitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'H');
                eosio_assert(hitcnt == 0, "Blackjack: action double1 cannot perform after hit");
                uint32_t doublecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'D');
                eosio_assert(doublecnt == 0, "Blackjack: action double1 cannot perform after double");
                uint32_t double1cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'T');
                eosio_assert(double1cnt == 0, "Blackjack: action double1 cannot perform twice");
                uint32_t standcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'S');
                eosio_assert(standcnt == 0, "Blackjack: action double1 cannot perform after stand");

                uint32_t hit1cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Q');
                eosio_assert(hit1cnt == 0, "Blackjack: action double1 cannot perform after hit1");
                uint32_t stand1cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'W');
                eosio_assert(stand1cnt == 0, "Blackjack: action double1 cannot perform after stand1");
                uint32_t hit2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'E');
                eosio_assert(hit2cnt == 0, "Blackjack: action double1 cannot perform after hit2");
                uint32_t stand2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'R');
                eosio_assert(stand2cnt == 0, "Blackjack: action double1 cannot perform after stand2");
                uint32_t double2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'A');
                eosio_assert(double2cnt == 0, "Blackjack: action double1 cannot perform after double2");

                uint32_t splitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Y');
                eosio_assert(splitcnt == 1, "Blackjack: action double1 must follow split");

                eosio_assert((itr_bjpool->bet / 2) == t.quantity.amount, "Blackjack: double1 must deposit the same amount as wager");
                bjpools.modify(itr_bjpool, _self, [&](auto &p){
                    p.actions = p.actions + "T";
                    p.bet = t.quantity.amount * 3;
                });
            } else if (actionid == 6) {
                bjstatus = 1;   // double hand2

                uint32_t double1cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'T');
                eosio_assert(double1cnt == 0 || double1cnt == 1, "Blackjack: action double1 can only perform zero or once");

                uint32_t hitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'H');
                eosio_assert(hitcnt == 0, "Blackjack: action double2 cannot perform after hit");
                uint32_t doublecnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'D');
                eosio_assert(doublecnt == 0, "Blackjack: action double2 cannot perform after double");
                uint32_t standcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'S');
                eosio_assert(standcnt == 0, "Blackjack: action double2 cannot perform after stand");

                uint32_t hit2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'E');
                eosio_assert(hit2cnt == 0, "Blackjack: action double2 cannot perform after hit2");
                uint32_t stand2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'R');
                eosio_assert(stand2cnt == 0, "Blackjack: action double2 cannot perform after stand2");
                uint32_t double2cnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'A');
                eosio_assert(double2cnt == 0, "Blackjack: action double2 cannot perform twice");

                uint32_t splitcnt = std::count(itr_bjpool->actions.begin(), itr_bjpool->actions.end(), 'Y');
                eosio_assert(splitcnt == 1, "Blackjack: action double2 must follow split");

                eosio_assert((itr_bjpool->bet / (2 + double1cnt)) == t.quantity.amount, "Blackjack: double2 must deposit the same amount as wager");
                bjpools.modify(itr_bjpool, _self, [&](auto &p){
                    p.actions = p.actions + "A";
                    p.bet = t.quantity.amount * (3 + double1cnt);
                });
            }

            bjpools.modify(itr_bjpool, _self, [&](auto &p) {
                p.status = bjstatus;
            });
        }
    } else if (gameid == 3) {
        uint32_t actionid = 0;
        int actionidx = usercomment.find("action[");
        if (actionidx >= 0) {
            int actionpos = usercomment.find("]", actionidx);
            if (actionpos > 0) {
                string ucmaction = usercomment.substr(actionidx + 7, actionpos - actionidx - 7);
                actionid = stoi(ucmaction);
            }
        }
        eosio_assert(actionid == 1 || actionid == 2 || actionid == 3 || actionid == 4 || actionid == 5, "UTH: invalid action id.");

        auto itr_uthpool = uthpools.find(user);

        if (actionid == 1) {
            if (bettoken == "EOS") {
                eosio_assert(t.quantity.amount >= 2000, "UTH: Below minimum bet threshold!");
                eosio_assert(t.quantity.amount <= 10000000, "UTH:Exceeds bet cap!");
            }
            eosio_assert(userseed.length() > 0, "user seed cannot by empty.");
            uint32_t nonce = increment_uthnonce(name{user});
            eosio_assert(itr_uthpool == uthpools.end(), "Ultimate Texas Holdem: your last round is not finished. Please contact admin!");

            int anteidx = usercomment.find("A[");
            eosio_assert(anteidx >= 0, "Ultimate Texas Holdem: cannot find ante in memo");
            int antepos = usercomment.find("]", anteidx);
            eosio_assert(antepos > 0, "Ultimate Texas Holdem: cannot find ante in memo");
            string antestr = usercomment.substr(anteidx + 2, antepos - anteidx - 2);
            uint64_t antenum = stoi(antestr);

            int tripidx = usercomment.find("T[");
            eosio_assert(tripidx >= 0, "Ultimate Texas Holdem: cannot find trip in memo");
            int trippos = usercomment.find("]", tripidx);
            eosio_assert(trippos > 0, "Ultimate Texas Holdem: cannot find trip in memo");
            string tripstr = usercomment.substr(tripidx + 2, trippos - tripidx - 2);
            uint64_t tripnum = stoi(tripstr);

            eosio_assert((antenum*2 + tripnum) == t.quantity.amount, "Ultimate Texas Holdem: amount does not match");

            uthpools.emplace(_self, [&](auto &p) {
                p.owner = name{user};
                p.status = 2;
                p.nonce = nonce;
                p.seed = userseed;
                p.bettoken = bettoken;
                p.bet = t.quantity.amount;
                p.ante = antenum;
                p.trip = tripnum;
            });

            if (bettoken == "EOS") {
                payref(name{user}, t.quantity.amount, 1, 1);
            }
        } else if (actionid == 2) {
            // triple
            eosio_assert(itr_uthpool->ante * 3 == t.quantity.amount, "Ultimate Texas Holdem: play must be 3 times as ante");
            eosio_assert(itr_uthpool->status == 2, "Ultimate Texas Holdem: previous status is not 2");

            uthpools.modify(itr_uthpool, _self, [&](auto &p){
                p.status = 1;
                p.actions = p.actions + "T";
                p.play = t.quantity.amount;
                p.bet += t.quantity.amount;
            });
        } else if (actionid == 3) {
            // x4
            eosio_assert(itr_uthpool->ante * 4 == t.quantity.amount, "Ultimate Texas Holdem: play must be 4 times as ante");
            eosio_assert(itr_uthpool->status == 2, "Ultimate Texas Holdem: previous status is not 2");

            uthpools.modify(itr_uthpool, _self, [&](auto &p){
                p.status = 1;
                p.actions = p.actions + "Q";
                p.play = t.quantity.amount;
                p.bet += t.quantity.amount;
            });
        } else if (actionid == 4) {
            //x2
            eosio_assert(itr_uthpool->ante * 2 == t.quantity.amount, "Ultimate Texas Holdem: play must be 2 times as ante");
            eosio_assert(itr_uthpool->status == 3, "Ultimate Texas Holdem: previous status is not 3");

            uthpools.modify(itr_uthpool, _self, [&](auto &p){
                p.status = 1;
                p.actions = p.actions + "D";
                p.play = t.quantity.amount;
                p.bet += t.quantity.amount;
            });

        } else if (actionid == 5) {
            //call
            eosio_assert(itr_uthpool->ante == t.quantity.amount, "Ultimate Texas Holdem: play must be equal to ante");
            eosio_assert(itr_uthpool->status == 4, "Ultimate Texas Holdem: previous status is not 4");

            uthpools.modify(itr_uthpool, _self, [&](auto &p){
                p.status = 1;
                p.actions = p.actions + "A";
                p.play = t.quantity.amount;
                p.bet += t.quantity.amount;
            });
        }

    } else if (gameid == 4){
        //pokerdice
        auto itr_pdpool = pdpools.find(user);

        if (bettoken == "EOS") {
            eosio_assert(t.quantity.amount >= 2000, "PokerDice: Below minimum bet threshold!");
            eosio_assert(t.quantity.amount <= 10000000, "PokerDice:Exceeds bet cap!");
        }
        eosio_assert(userseed.length() > 0, "user seed cannot by empty.");
        uint32_t nonce = increment_pdnonce(name{user});
        eosio_assert(itr_pdpool == pdpools.end(), "PokerDice: your last round is not finished. Please contact admin!");


        string bet_cards = "";
        string bet_value = "";

        uint32_t betcardidx = usercomment.find("bet_cards[");
        if (betcardidx > 0 && betcardidx != 4294967295)
        {
            uint32_t betcardpos = usercomment.find("]", betcardidx);
            if (betcardpos > 0 && betcardpos != 4294967295)
            {
                bet_cards = usercomment.substr(betcardidx + 10, betcardpos - betcardidx - 10);
            }
        }

        uint32_t valueidx = usercomment.find("bet_value[");
        if (valueidx > 0 && valueidx != 4294967295)
        {
            uint32_t valuepos = usercomment.find("]", valueidx);
            if (valuepos > 0 && valuepos != 4294967295)
            {
                bet_value = usercomment.substr(valueidx + 10, valuepos - valueidx - 10);
            }
        }


        pdpools.emplace(_self, [&](auto &p) {
            p.owner = name{user};
            p.status = 1;
            p.nonce = nonce;
            p.seed = userseed;
            p.bettoken = bettoken;
            p.betcards = bet_cards;
            p.betvalue = bet_value;
            p.bet = t.quantity.amount;
        });

        if (bettoken == "EOS") {
            payref(name{user}, t.quantity.amount, 1, 1);
        }

    } else if (gameid == 88) {
        if (bettoken == "EOS" && t.quantity.amount <= 10000) {
            asset bal = asset(t.quantity.amount, symbol_type(S(4, EOS)));
            if (bal.amount > 0) {
                // withdraw
                action(permission_level{_self, N(active)}, N(eosio.token),
                       N(transfer), std::make_tuple(_self, user, bal,
                                                    std::string("www.rovegas.com")))
                        .send();
            }
        }
        return;
    }

    if (bettoken == "EOS") {
        // update user exp and level
        auto itr_paccount = paccounts.find(user);
        uint32_t nextlev = 0;
        uint32_t prevlev = itr_paccount->level;
        uint64_t amount = t.quantity.amount;
        if (itr_paccount == paccounts.end()) {
            uint64_t newexp = amount * expbygame[gameid];
            nextlev = getlevel(newexp);
            paccounts.emplace(_self, [&](auto &p){
                p.owner = name{user};
                p.level = nextlev;
                p.exp = newexp;
            });
        } else {
            uint64_t newexp = itr_paccount->exp + amount * expbygame[gameid];

            nextlev = getlevel(newexp);
            paccounts.modify(itr_paccount, _self, [&](auto &p) {
                p.level = nextlev;
                p.exp = newexp;
            });
        }
        /*
        // send new level 1 user 50 mev
        if (prevlev == 0 && nextlev == 1) {
            asset ballev1 = asset(500000, symbol_type(S(4, MEV)));
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(N(eosvegasjack), user, ballev1,
                                                std::string("Congratulations on your level 1! Please enjoy the game! - ROVegas.com")))
                    .send();
        }
         */
    }

    /*
    // TODO
    // videopoker promo
    auto itr_jackevent = jackevents.find(user);
    if (itr_jackevent == jackevents.end()) {
        itr_jackevent = jackevents.emplace(_self, [&](auto &p){
            p.owner = name{user};
            p.eosin = amount;
        });
    } else {
        jackevents.modify(itr_jackevent, _self, [&](auto &p) {
            p.eosin += amount;
        });
    }
    */
}



void pokergame1::uthcheck(const name player, uint32_t nonce, std::vector<uint32_t> common_hand, std::vector<uint32_t> player_hand, uint32_t cur_status) {
    require_auth(player);

    auto itr_uthpool = uthpools.find(player);
    eosio_assert(itr_uthpool != uthpools.end(), "UTH: cannot find uthpool");
    eosio_assert(cur_status == 2 || cur_status == 3, "UTH: invalid status");

    if (cur_status == 2) {
        eosio_assert(itr_uthpool->status == 2, "UTH: status does not match for cur status 2");

        uthpools.modify(itr_uthpool, _self, [&](auto &p){
            p.status = 3;
            p.actions = p.actions + "C";
        });
    } else {
        eosio_assert(itr_uthpool->status == 3, "UTH: status does not match for cur status 3");

        uthpools.modify(itr_uthpool, _self, [&](auto &p){
            p.status = 4;
            p.actions = p.actions + "C";
        });
    }


}

void pokergame1::uthfold(const name player, uint32_t nonce, std::vector<uint32_t> common_hand, std::vector<uint32_t> player_hand) {

    require_auth(player);

    auto itr_uthpool = uthpools.find(player);
    eosio_assert(itr_uthpool != uthpools.end(), "UTH: cannot find uthpool");
    eosio_assert(itr_uthpool->status == 4, "UTH: status does not match for cur status 4");

    uthpools.modify(itr_uthpool, _self, [&](auto &p){
        p.status = 1;
        p.actions = p.actions + "F";
    });
}

void pokergame1::uthreceipt(string game_id, const name player, string game, string seed, std::vector<string> dealer_hand,
                std::vector<string> common_hand, std::vector<string> player_hand, string ante, string blind, string trip,
                string play, string ante_win, string blind_win, string trip_win, string play_win, string dealer_win_type,
                string player_win_type, uint64_t betnum, uint64_t winnum, string token, string actions, string pub_key) {
    require_auth(N(eosvegasjack));
    require_recipient(player);

    auto itr_uthpool = uthpools.find(player);
    auto itr_metadata = metadatas.find(0);
    eosio_assert(itr_uthpool != uthpools.end(), "UTH: cannot find uthpool");
    eosio_assert(itr_uthpool->status == 1, "UTH: status should be 1");
    eosio_assert(itr_uthpool->bet > 0 && itr_uthpool->ante > 0, "UTH:bet and ante must be larger than zero");

    int pos1 = seed.find("_");
    eosio_assert(pos1 > 0, "UTH: seed is incorrect.");
    string ucm = seed.substr(0, pos1);
    uint32_t seednonce = stoi(ucm);
    eosio_assert(seednonce == itr_uthpool->nonce, "UTH: nonce does not match.");

    eosio_assert(betnum == itr_uthpool->bet, "UTH: bet does not match.");
    eosio_assert(token == itr_uthpool->bettoken, "UTH: bet token does not match.");

    if (token == "EOS") {
        auto itr_paccount = paccounts.find(player);
        eosio_assert(itr_paccount != paccounts.end(), "UTH: user not found");

        asset bal = asset(winnum, symbol_type(S(4, EOS)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosio.token),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- texas.rovegas.com")))
                    .send();
        }

        uint64_t mineprice = getminingtableprice(itr_metadata->tmevout);
        uint64_t minemev = betnum * mineprice * (1 + itr_paccount->level * 0.05) * minebygame[3] / (100*100);

        report(player, minemev, betnum, winnum);
        //updatemevout(player, itr_bjpool->nonce, minemev, 2);

        asset bal2 = asset(minemev, symbol_type(S(4, MEV)));
        if (bal2.amount > 0) {
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(N(eosvegasjack), player, bal2,
                                                std::string("Gaming deserves rewards! - texas.rovegas.com")))
                    .send();
        }
    } else if (token == "MEV") {
        asset bal = asset(winnum, symbol_type(S(4, MEV)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- texas.rovegas.com")))
                    .send();
        }
    }

    uthpools.erase(itr_uthpool);



}


void pokergame1::pdreceipt(string game_id, const name player, string game, string seed, string bet_result,
                           string bet_cards, string bet_value, uint64_t betnum, uint64_t winnum, string token,
                           string pub_key) {
    require_auth(N(eosvegasjack));
    require_recipient(player);

    auto itr_pdpool = pdpools.find(player);
    auto itr_metadata = metadatas.find(0);
    eosio_assert(itr_pdpool != pdpools.end(), "PokerDice: cannot find uthpool");
    eosio_assert(itr_pdpool->status == 1, "PokerDice: status should be 1");
    eosio_assert(itr_pdpool->bet == betnum, "PokerDice: bet amount does not match");

    int pos1 = seed.find("_");
    eosio_assert(pos1 > 0, "PokerDice: seed is incorrect.");
    string ucm = seed.substr(0, pos1);
    uint32_t seednonce = stoi(ucm);
    eosio_assert(seednonce == itr_pdpool->nonce, "PokerDice: nonce does not match.");

    eosio_assert(token == itr_pdpool->bettoken, "PokerDice: bet token does not match.");

    if (token == "EOS") {
        auto itr_paccount = paccounts.find(player);
        eosio_assert(itr_paccount != paccounts.end(), "PokerDice: user not found");

        asset bal = asset(winnum, symbol_type(S(4, EOS)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosio.token),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- pokerdice.rovegas.com")))
                    .send();
        }

        uint64_t mineprice = getminingtableprice(itr_metadata->tmevout);
        uint64_t minemev = betnum * mineprice * (1 + itr_paccount->level * 0.05) * minebygame[3] / (100*100);

        report(player, minemev, betnum, winnum);

        asset bal2 = asset(minemev, symbol_type(S(4, MEV)));
        if (bal2.amount > 0) {
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(N(eosvegasjack), player, bal2,
                                                std::string("Gaming deserves rewards! - pokerdice.rovegas.com")))
                    .send();
        }
    } else if (token == "MEV") {
        asset bal = asset(winnum, symbol_type(S(4, MEV)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- pokerdice.rovegas.com")))
                    .send();
        }
    }

    pdpools.erase(itr_pdpool);
}

void pokergame1::report(name from, uint64_t minemev, uint64_t meosin, uint64_t meosout) {
    auto itr_metadata = metadatas.find(0);
    metadatas.modify(itr_metadata, _self, [&](auto &p) {
        p.tmevout += minemev;
        p.teosin += meosin;
        p.teosout += (meosout + meosin*0.001);
        p.trounds += 1;
    });
/*
    uint32_t reportidx = 0;
    if (gameid == 0 || gameid == 1) {
        reportidx = 2;
    } else if (gameid == 2) {
        reportidx = 1;
    }
    eosio_assert(reportidx != 0, "Incorrect game reporting. Please contact admin.");

    auto itr_metadata2 = metadatas.find(reportidx);
    metadatas.modify(itr_metadata2, _self, [&](auto &p) {
        p.tmevout += minemev;
        p.teosin += meosin;
        p.teosout += (meosout + meosin*0.005);
        p.trounds += 1;
    });


    // update ginfo table for global reporting
    uint32_t cur_in_sec = now();
    uint32_t month_in_sec = getstartmonth(cur_in_sec);
    uint32_t week_in_sec = getstartweek(cur_in_sec);
    uint32_t day_in_sec = getstartday(cur_in_sec);
    auto itr_ginfo = ginfos.find(month_in_sec);
    if (itr_ginfo == ginfos.end()) {
        itr_ginfo = ginfos.emplace(_self, [&](auto &p){
            p.startmonth = month_in_sec;
            p.tmevout = 0;
            p.teosin = 0;
            p.teosout = 0;
        });
    }
    ginfos.modify(itr_ginfo, _self, [&](auto &p) {
        p.tmevout += minemev;
        p.teosin += meosin;
        p.teosout += meosout;
    });
*/
}

void pokergame1::vpdraw(const name from, uint32_t nonce, std::vector<string> actions) {
    require_auth(from);

    auto itr_vppool = vppools.find(from);
    eosio_assert(itr_vppool != vppools.end(), "Jacks-or-Better: user cannot be found. Please contact us for assistance!");
    eosio_assert(itr_vppool->status == 1, "Jacks-or-Better: wrong status. Please contact us for assistance!");
    eosio_assert(itr_vppool->nonce == nonce, "Jacks-or-Better: nonce doesn't match.");

    vppools.modify(itr_vppool, _self, [&](auto &p) {
        p.status = 2;
        p.card1 = actions[0];
        p.card2 = actions[1];
        p.card3 = actions[2];
        p.card4 = actions[3];
        p.card5 = actions[4];
    });

    /*
    // update the drawn card in cardstats
    auto itr_cardstat = cardstats.find(card % 52);
    eosio_assert(itr_cardstat != cardstats.end(), "Cannot find the card in card statistic table.");
    cardstats.modify(itr_cardstat, _self, [&](auto &p) {
        p.count += 1;
    });
     */

    // clear mevouts;
    auto itr_mevout = mevouts.begin();
    uint32_t cnt = 0;
    while (itr_mevout != mevouts.end() && cnt < 16) {
        if (itr_mevout->lastseen + 3600 < now()) {
            itr_mevout = mevouts.erase(itr_mevout);
        } else {
            itr_mevout++;
        }
        cnt++;
    }
}

void pokergame1::updatemevout(name player, uint32_t nonce, uint64_t mevout, uint32_t mode) {
    if (mode == 1) {
        auto itr = mevouts.find(player);
        if (itr == mevouts.end()) {
            mevouts.emplace(_self, [&](auto &p){
                p.owner = player;
                p.nonce = nonce;
                p.mevout = mevout;
                p.lastseen = now();
            });
        } else {
            mevouts.modify(itr, _self, [&](auto &p) {
                p.owner = player;
                p.nonce = nonce;
                p.mevout = mevout;
                p.lastseen = now();
            });
        }
    } else if (mode == 2) {
        auto itr = bjmevouts.find(player);
        if (itr == bjmevouts.end()) {
            bjmevouts.emplace(_self, [&](auto &p){
                p.owner = player;
                p.nonce = nonce;
                p.mevout = mevout;
                p.lastseen = now();
            });
        } else {
            bjmevouts.modify(itr, _self, [&](auto &p) {
                p.owner = player;
                p.nonce = nonce;
                p.mevout = mevout;
                p.lastseen = now();
            });
        }
    }

}

void pokergame1::vpreceipt(string game_id, const name player, string game, std::vector<string> player_hand,
        string bet, string win, string wintype, string seed, string dealer_signature, uint64_t betnum, uint64_t winnum,
        string token, string pub_key, uint64_t event_win) {

    require_auth(N(eosvegasjack));

    require_recipient(player);
    //sanity_check(N(eosvegasjack), N(vpreceipt));

    auto itr_vppool = vppools.find(player);
    auto itr_metadata = metadatas.find(0);

    eosio_assert(itr_vppool != vppools.end(), "Jacks-or-Better:User pool not found");
    eosio_assert(itr_vppool->mode == 0, "Jacks-or-Better: wrong game mode");

    eosio_assert(itr_vppool->bet > 0, "Jacks-or-Better:bet must be larger than zero");
    eosio_assert(itr_vppool->status == 2, "Jacks-or-Better: wrong status. Please contact admin.");

    eosio_assert(betnum == itr_vppool->bet, "vpreceipt: bet does not match.");
    eosio_assert(token == itr_vppool->bettoken, "vpreceipt: bet token does not match.");

    uint32_t pos1 = seed.find("_");
    eosio_assert(pos1 > 0 && pos1 != 4294967295, "vpreceipt: seed is incorrect.");
    if (pos1 > 0) {
        string ucm = seed.substr(0, pos1);

        uint32_t seednonce = stoi(ucm);
        eosio_assert(seednonce == itr_vppool->nonce, "vpreceipt: nonce does not match.");
    }

    /*
    //update big wins
    uint32_t cards[5];
    for(std::vector<string>::size_type i = 0; i != player_hand.size(); i++) {
        uint32_t pos = player_hand[i].find("[");
        eosio_assert(pos > 0 && pos1 != 4294967295, "Card is incorrect");
        string ucm = player_hand[i].substr(0, pos);
        cards[i] = stoi(ucm);
    }

    uint32_t type = checkwin(cards[0], cards[1], cards[2], cards[3], cards[4]);
    if (token == "EOS") {
        //payref(player, itr_vppool->bet, 1, 6);

        auto itr_metadatag2 = metadatas.find(2);
        if (type >= 4) {
            events.emplace(_self, [&](auto &p){
                p.id = events.available_primary_key();
                p.owner = player;
                p.datetime = now();
                p.wintype = type;
                p.ratio = ratios[type];
                p.bet = betnum;
                p.betwin = winnum;
                p.card1 = cards[0];
                p.card2 = cards[1];
                p.card3 = cards[2];
                p.card4 = cards[3];
                p.card5 = cards[4];
            });
            eosio_assert(itr_metadatag2 != metadatas.end(), "Metadata is empty.");
            metadatas.modify(itr_metadatag2, _self, [&](auto &p){
                p.eventcnt = p.eventcnt + 1;
            });
        }

        while (itr_metadatag2->eventcnt > 32) {
            auto itr_event2 = events.begin();
            events.erase(itr_event2);
            metadatas.modify(itr_metadatag2, _self, [&](auto &p){
                p.eventcnt = p.eventcnt - 1;
            });
        }
    }
    */

    /*
    //jackpot
    int jackpot_idx = wintype.find("X250");
    if (jackpot_idx >= 0 && token == "EOS") {
        auto itr_pevent = pevents.find(0);
        uint64_t tempam = 0;
        if (itr_vppool->bet >= 50000) {
            tempam = itr_pevent->eosin * 0.9;
            puevents.emplace(_self, [&](auto &p) {
                p.id = puevents.available_primary_key();
                p.owner = player;
                p.type = 1;
            });
        } else if (itr_vppool->bet >= 10000 && itr_vppool->bet< 50000) {
            tempam = itr_pevent->eosin * 0.18;
            puevents.emplace(_self, [&](auto &p) {
                p.id = puevents.available_primary_key();
                p.owner = player;
                p.type = 2;
            });
        } else if (itr_vppool->bet >= 2500 && itr_vppool->bet < 10000) {
            tempam = itr_pevent->eosin * 0.045;
            puevents.emplace(_self, [&](auto &p) {
                p.id = puevents.available_primary_key();
                p.owner = player;
                p.type = 3;
            });
        }

        pevents.modify(itr_pevent, _self, [&](auto &p) {
            p.eosin = p.eosin - tempam;
        });
        winnum += tempam;
    }
*/

    if (token == "EOS") {
        auto itr_paccount = paccounts.find(player);
        eosio_assert(itr_paccount != paccounts.end(), "Jacks-or-Better:User not found");

        winnum = winnum + event_win;

        asset bal = asset(winnum, symbol_type(S(4, EOS)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosio.token),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- jacks.rovegas.com")))
                    .send();
        }

        uint64_t mineprice = getminingtableprice(itr_metadata->tmevout);
        uint64_t minemev = betnum * mineprice * (1 + itr_paccount->level * 0.05) / 100;

        report(player, minemev, betnum, winnum);
        updatemevout(player, itr_vppool->nonce, minemev, 1);

        asset bal2 = asset(minemev, symbol_type(S(4, MEV)));
        if (bal2.amount > 0) {
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(N(eosvegasjack), player, bal2,
                                                std::string("Gaming deserves rewards! - jacks.rovegas.com")))
                    .send();
        }
    } else if (token == "MEV") {
        asset bal = asset(winnum, symbol_type(S(4, MEV)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- jacks.rovegas.com")))
                    .send();
        }
    } else if (token == "EUSD") {
        asset bal = asset(winnum, symbol_type(S(8, EUSD)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(bitpietokens),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- jacks.rovegas.com")))
                    .send();
        }
    }

    vppools.erase(itr_vppool);
}

void pokergame1::vp5xreceipt(string game_id, const name player, string game, std::vector<string> player_hand1,
        std::vector<string> player_hand2, std::vector<string> player_hand3, std::vector<string> player_hand4,
        std::vector<string> player_hand5, string bet, string win, string wintype, string seed, string dealer_signature,
        uint64_t betnum, uint64_t winnum, string token, string pub_key, uint64_t event_win) {

    require_auth(N(eosvegasjack));

    require_recipient(player);
    //sanity_check(N(eosvegasjack), N(vpreceipt));

    auto itr_vppool = vppools.find(player);
    auto itr_metadata = metadatas.find(0);

    eosio_assert(itr_vppool != vppools.end(), "Jacks-or-Better:User pool not found");
    eosio_assert(itr_vppool->mode == 1, "Jacks-or-Better: wrong game mode");

    eosio_assert(itr_vppool->bet > 0, "Jacks-or-Better:bet must be larger than zero");
    eosio_assert(itr_vppool->status == 2, "Jacks-or-Better: wrong status. Please contact admin.");

    uint32_t pos1 = seed.find("_");
    eosio_assert(pos1 > 0 && pos1 != 4294967295, "vp5xreceipt: seed is incorrect.");
    if (pos1 > 0) {
        string ucm = seed.substr(0, pos1);

        uint32_t seednonce = stoi(ucm);
        eosio_assert(seednonce == itr_vppool->nonce, "vp5xreceipt: nonce does not match.");
    }

    eosio_assert(betnum == itr_vppool->bet, "vp5xreceipt: bet does not match.");
    eosio_assert(token == itr_vppool->bettoken, "vp5xreceipt: bet token does not match.");

    /*
    uint32_t isjackpot = 0;
    if (token == "EOS") {
        //payref(player, itr_vppool->bet, 1, 6);


        // update wins
        auto itr_metadatag2 = metadatas.find(2);
        for (int i = 0; i < 5; i++) {
            std::vector<string> player_hand;
            if (i == 0) {
                player_hand = player_hand1;
            } else if (i == 1) {
                player_hand = player_hand2;
            } else if (i == 2) {
                player_hand = player_hand3;
            } else if (i == 3) {
                player_hand = player_hand4;
            } else if (i == 4) {
                player_hand = player_hand5;
            }

            //update big wins
            uint32_t cards[5];
            for(std::vector<string>::size_type i = 0; i != player_hand.size(); i++) {
                uint32_t pos = player_hand[i].find("[");
                eosio_assert(pos > 0 && pos1 != 4294967295, "Card is incorrect");
                string ucm = player_hand[i].substr(0, pos);
                cards[i] = stoi(ucm);
            }
            uint32_t type = checkwin(cards[0], cards[1], cards[2], cards[3], cards[4]);
            if (type >= 4) {
                events.emplace(_self, [&](auto &p){
                    p.id = events.available_primary_key();
                    p.owner = player;
                    p.datetime = now();
                    p.wintype = type;
                    p.ratio = ratios[type];
                    p.bet = (betnum / 5);
                    p.betwin = (betnum / 5) * ratios[type];
                    p.card1 = cards[0];
                    p.card2 = cards[1];
                    p.card3 = cards[2];
                    p.card4 = cards[3];
                    p.card5 = cards[4];
                });
                eosio_assert(itr_metadatag2 != metadatas.end(), "Metadata is empty.");
                metadatas.modify(itr_metadatag2, _self, [&](auto &p){
                    p.eventcnt = p.eventcnt + 1;
                });
            }

            if (type == 9) {
                isjackpot = 1;
            }
        }
        while (itr_metadatag2->eventcnt > 32) {
            auto itr_event2 = events.begin();
            events.erase(itr_event2);
            metadatas.modify(itr_metadatag2, _self, [&](auto &p){
                p.eventcnt = p.eventcnt - 1;
            });
        }
    }
    */


    /*
    int jackpot_idx = wintype.find("X250");
    if (jackpot_idx >= 0 && token == "EOS") {
        auto itr_pevent = pevents.find(0);
        uint64_t tempam = 0;
        if (itr_vppool->bet >= 250000) {
            tempam = itr_pevent->eosin * 0.9;
            puevents.emplace(_self, [&](auto &p) {
                p.id = puevents.available_primary_key();
                p.owner = player;
                p.type = 1;
            });
        } else if (itr_vppool->bet >= 50000 && itr_vppool->bet< 250000) {
            tempam = itr_pevent->eosin * 0.18;
            puevents.emplace(_self, [&](auto &p) {
                p.id = puevents.available_primary_key();
                p.owner = player;
                p.type = 2;
            });
        } else if (itr_vppool->bet >= 12500 && itr_vppool->bet < 50000) {
            tempam = itr_pevent->eosin * 0.045;
            puevents.emplace(_self, [&](auto &p) {
                p.id = puevents.available_primary_key();
                p.owner = player;
                p.type = 3;
            });
        }

        pevents.modify(itr_pevent, _self, [&](auto &p) {
            p.eosin = p.eosin - tempam;
        });
        winnum += tempam;
    }
     */

    if (token == "EOS") {
        auto itr_paccount = paccounts.find(player);
        eosio_assert(itr_paccount != paccounts.end(), "Jacks-or-Better:User not found");

        winnum = winnum + event_win;

        asset bal = asset(winnum, symbol_type(S(4, EOS)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosio.token),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- jacks.rovegas.com")))
                    .send();
        }

        uint64_t mineprice = getminingtableprice(itr_metadata->tmevout);
        uint64_t minemev = betnum * mineprice * (1 + itr_paccount->level * 0.05) / 100;

        report(player, minemev, betnum, winnum);
        updatemevout(player, itr_vppool->nonce, minemev, 1);

        asset bal2 = asset(minemev, symbol_type(S(4, MEV)));
        if (bal2.amount > 0) {
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(N(eosvegasjack), player, bal2,
                                                std::string("Gaming deserves rewards! - jacks.rovegas.com")))
                    .send();
        }
    } else if (token == "MEV") {
        asset bal = asset(winnum, symbol_type(S(4, MEV)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosvegascoin),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- jacks.rovegas.com")))
                    .send();
        }
    } else if (token == "EUSD") {
        asset bal = asset(winnum, symbol_type(S(8, EUSD)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(bitpietokens),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Winner winner chicken dinner! 大吉大利，今晚吃鸡！- jacks.rovegas.com")))
                    .send();
        }
    }

    vppools.erase(itr_vppool);
}


uint32_t pokergame1::checkwin(uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t c5) {
    uint32_t cards[5];
    uint32_t colors[5];
    uint32_t numbers[5];
    cards[0] = c1;
    cards[1] = c2;
    cards[2] = c3;
    cards[3] = c4;
    cards[4] = c5;
    sort(cards, cards + 5);

    colors[0] = cards[0] / 13;
    numbers[0] = cards[0] % 13;
    colors[1] = cards[1] / 13;
    numbers[1] = cards[1] % 13;
    colors[2] = cards[2] / 13;
    numbers[2] = cards[2] % 13;
    colors[3] = cards[3] / 13;
    numbers[3] = cards[3] % 13;
    colors[4] = cards[4] / 13;
    numbers[4] = cards[4] % 13;
    sort(numbers, numbers + 5);

    uint32_t finalratio = 0;
    uint32_t type = 0;
    if (numbers[0] == 0 && numbers[1] == 9 && numbers[2] == 10 && numbers[3] == 11 && numbers[4] == 12 && checkflush(colors)) {
        // Royal Flush A, 10, 11, 12, 13
        finalratio = ratios[9];
        type = 9;
    } else if (checkstraight(numbers) && checkflush(colors)) {
        // straight flush
        finalratio = ratios[8];
        type = 8;
    } else if (checksame(numbers, 4) == 1) {
        finalratio = ratios[7];
        type = 7;
    } else if (checksame(numbers, 3) == 1 && checksame(numbers, 2) == 1) {
        finalratio = ratios[6];
        type = 6;
    } else if (checkflush(colors)) {
        finalratio = ratios[5];
        type = 5;
    } else if (checkstraight(numbers)) {
        finalratio = ratios[4];
        type = 4;
    } else if (checksame(numbers, 3) == 1) {
        finalratio = ratios[3];
        type = 3;
    } else if (checksame(numbers, 2) == 2) {
        finalratio = ratios[2];
        type = 2;
    } else if (checkBiggerJack(numbers)) {
        finalratio = ratios[1];
        type = 1;
    } else {
        finalratio = ratios[0];
        type = 0;
    }
    return type;
}
/*
void pokergame1::drawcards5x(const name from, uint32_t externalsrc, string dump1, string dump2, string dump3, string dump4, string dump5) {
    require_auth(from);
    sanity_check(N(eosvegasjack), N(drawcards5x));

    auto itr_blacklist = blacklists.find(from);
    eosio_assert(itr_blacklist == blacklists.end(), "Sorry, please be patient.");

    auto itr_pool = pools.find(from);
    eosio_assert(itr_pool != pools.end(), "User not found");
    eosio_assert(itr_pool->cardhash1.length() != 0, "Cards hasn't bee drawn.");
    eosio_assert(itr_pool->bet > 0, "Bet must be larger than zero.");
    eosio_assert(itr_pool->cardhash2.length() == 0, "New cards already assigned.");
    eosio_assert(parsecard(dump1) == itr_pool->card1, "card1 mismatch");
    eosio_assert(parsecard(dump2) == itr_pool->card2, "card2 mismatch");
    eosio_assert(parsecard(dump3) == itr_pool->card3, "card3 mismatch");
    eosio_assert(parsecard(dump4) == itr_pool->card4, "card4 mismatch");
    eosio_assert(parsecard(dump5) == itr_pool->card5, "card5 mismatch");
    eosio_assert(itr_pool->status == 1, "Wrong game mode!");

    auto itr_metadata = metadatas.find(2);
    auto itr_paccount = paccounts.find(from);


    std::set<uint32_t> myset;
    bool barr[5];
    barr[0] = ishold(dump1) == true;
    barr[1] = ishold(dump2) == true;
    barr[2] = ishold(dump3) == true;
    barr[3] = ishold(dump4) == true;
    barr[4] = ishold(dump5) == true;

    //checksum256 roothash = gethash(from, externalsrc, itr_metadata->trounds);
    checksum256 roothash = gethash(from, externalsrc, 0);
    string rhash;

    for( int i = 0; i < 32; ++i )
    {
        char const byte = roothash.hash[i];
        rhash += hex_chars[ ( byte & 0xF0 ) >> 4 ];
        rhash += hex_chars[ ( byte & 0x0F ) >> 0 ];
    }
    pools.modify(itr_pool, _self, [&](auto &p){
        p.cardhash2 = string(rhash);
    });


    // find how many new cards we need to draw
    uint32_t cnt = 0;
    for (int i = 0; i < 5; i++) {
        if (barr[i] != true) {
            cnt++;
        }
    }
    uint32_t newarr[cnt];   // new cards we final get for each round
    uint64_t eachbet = itr_pool->bet / 5;
    uint64_t ebetwin[5];
    uint64_t tbetwin = 0;
    uint64_t twintype = 0;
    string tresult;
    uint64_t newcards[5];
    string newcardsstr[5];

    // jackpot
    uint32_t isjackpot = 0;

    // 5 card sets
    for (int i = 0; i < 5; i++) {
        myset.clear();
        myset.insert(itr_pool->card1);
        myset.insert(itr_pool->card2);
        myset.insert(itr_pool->card3);
        myset.insert(itr_pool->card4);
        myset.insert(itr_pool->card5);

        getcards(from, roothash, newarr, cnt, myset, 0, 52);

        checksum256 newhash;
        sha256((char *)&roothash.hash, 32, &newhash);
        roothash = newhash;

        uint32_t newcnt = 0;

        uint32_t n1 = barr[0] == false ? newarr[newcnt++] : itr_pool->card1;
        newcards[i] |= n1;
        newcards[i] <<= 8;
        newcardsstr[i] = to_string(n1) + ", ";

        uint32_t n2 = barr[1] == false ? newarr[newcnt++] : itr_pool->card2;
        newcards[i] |= n2;
        newcards[i] <<= 8;
        newcardsstr[i] = newcardsstr[i] + to_string(n2) + ", ";

        uint32_t n3 = barr[2] == false ? newarr[newcnt++] : itr_pool->card3;
        newcards[i] |= n3;
        newcards[i] <<= 8;
        newcardsstr[i] = newcardsstr[i] + to_string(n3) + ", ";

        uint32_t n4 = barr[3] == false ? newarr[newcnt++] : itr_pool->card4;
        newcards[i] |= n4;
        newcards[i] <<= 8;
        newcardsstr[i] = newcardsstr[i] + to_string(n4) + ", ";

        uint32_t n5 = barr[4] == false ? newarr[newcnt++] : itr_pool->card5;
        newcards[i] |= n5;
        newcardsstr[i] = newcardsstr[i] + to_string(n5);

        uint32_t type = checkwin(n1, n2, n3, n4, n5);
        twintype <<= 8;
        twintype |= type;
        ebetwin[i] = eachbet * ratios[type];
        tbetwin += ebetwin[i];
        tresult = tresult + " X" + to_string(ratios[type]);


        //jackpot
        if (type == 9) {
            isjackpot = 1;
        }

        // records events if wintype >= straight
        if (type >= 4) {
            events.emplace(_self, [&](auto &p){
                p.id = events.available_primary_key();
                p.owner = from;
                p.datetime = now();
                p.wintype = type;
                p.ratio = ratios[type];
                p.bet = eachbet;
                p.betwin = ebetwin[i];
                p.card1 = n1;
                p.card2 = n2;
                p.card3 = n3;
                p.card4 = n4;
                p.card5 = n5;
            });
            eosio_assert(itr_metadata != metadatas.end(), "Metadata is empty.");
            metadatas.modify(itr_metadata, _self, [&](auto &p){
                p.eventcnt = p.eventcnt + 1;
            });
        }

        auto itr_typestat = typestats.find(type);
        typestats.modify(itr_typestat, _self, [&](auto &p) {
            p.count += 1;
        });
    }

    while (itr_metadata->eventcnt > 32) {
        auto itr_event2 = events.begin();
        events.erase(itr_event2);
        metadatas.modify(itr_metadata, _self, [&](auto &p){
            p.eventcnt = p.eventcnt - 1;
        });
    }

    auto itr_pool5x = pool5xs.find(from);
    if (itr_pool5x == pool5xs.end()) {
        itr_pool5x = pool5xs.emplace(_self, [&](auto &p){
            p.owner = from;
            p.cards1 = newcards[0];
            p.cards2 = newcards[1];
            p.cards3 = newcards[2];
            p.cards4 = newcards[3];
            p.cards5 = newcards[4];
            p.wintype =twintype;
            p.betwin1 = ebetwin[0];
            p.betwin2 = ebetwin[1];
            p.betwin3 = ebetwin[2];
            p.betwin4 = ebetwin[3];
            p.betwin5 = ebetwin[4];
        });
    } else {
        pool5xs.modify(itr_pool5x, _self, [&](auto &p){
            p.cards1 = newcards[0];
            p.cards2 = newcards[1];
            p.cards3 = newcards[2];
            p.cards4 = newcards[3];
            p.cards5 = newcards[4];
            p.wintype =twintype;
            p.betwin1 = ebetwin[0];
            p.betwin2 = ebetwin[1];
            p.betwin3 = ebetwin[2];
            p.betwin4 = ebetwin[3];
            p.betwin5 = ebetwin[4];
        });
    }

    //jackpot
    if (isjackpot == 1) {
        auto itr_pevent = pevents.find(0);
        uint64_t tempam = 0;
        if (itr_pool->bet >= 250000) {
            tempam = itr_pevent->eosin * 0.9;
            puevents.emplace(_self, [&](auto &p) {
                p.id = puevents.available_primary_key();
                p.owner = from;
                p.type = 1;
            });
        } else if (itr_pool->bet >= 50000 && itr_pool->bet < 250000) {
            tempam = itr_pevent->eosin * 0.18;
            puevents.emplace(_self, [&](auto &p) {
                p.id = puevents.available_primary_key();
                p.owner = from;
                p.type = 2;
            });
        } else if (itr_pool->bet >= 12500 && itr_pool->bet < 50000) {
            tempam = itr_pevent->eosin * 0.045;
            puevents.emplace(_self, [&](auto &p) {
                p.id = puevents.available_primary_key();
                p.owner = from;
                p.type = 3;
            });
        }

        pevents.modify(itr_pevent, _self, [&](auto &p) {
            p.eosin = p.eosin - tempam;
        });
        tbetwin += tempam;
    }

    auto itr_metadatatotal = metadatas.find(0);
    uint64_t mineprice = getminingtableprice(itr_metadatatotal->tmevout);
    uint64_t minemev = itr_pool->bet * mineprice * (1 + itr_paccount->level * 0.05) / 100;
    uint64_t meosin = itr_pool->bet;
    uint64_t meosout = tbetwin;

    // update here for receipt5x
    pools.modify(itr_pool, _self, [&](auto &p) {
        p.betwin = meosout;
    });

    report(from, minemev, meosin, meosout, 1);      // gameid =1

    //string cbet = to_string(meosin / 10000) + '.' + to_string(meosin % 10000) + " EOS";
    //string cbetwin = to_string(meosout / 10000) + '.' + to_string(meosout % 10000) + " EOS";
}
 */
/*
void pokergame1::drawcards(const name from, uint32_t externalsrc, string dump1, string dump2, string dump3, string dump4, string dump5) {
    require_auth(from);

    sanity_check(N(eosvegasjack), N(drawcards));

    auto itr_blacklist = blacklists.find(from);
    eosio_assert(itr_blacklist == blacklists.end(), "Sorry, please be patient.");

    auto itr_user = pools.find(from);
    eosio_assert(itr_user != pools.end(), "User not found");
    eosio_assert(itr_user->cardhash1.length() != 0, "Cards hasn't bee drawn.");
    eosio_assert(itr_user->bet > 0, "Bet must be larger than zero.");
    eosio_assert(itr_user->cardhash2.length() == 0, "New cards already assigned.");
    eosio_assert(parsecard(dump1) == itr_user->card1, "card1 mismatch");
    eosio_assert(parsecard(dump2) == itr_user->card2, "card2 mismatch");
    eosio_assert(parsecard(dump3) == itr_user->card3, "card3 mismatch");
    eosio_assert(parsecard(dump4) == itr_user->card4, "card4 mismatch");
    eosio_assert(parsecard(dump5) == itr_user->card5, "card5 mismatch");
    eosio_assert(itr_user->status == 0, "Wrong game mode!");

    auto itr_metadata = metadatas.find(2);
    auto itr_paccount = paccounts.find(from);
    bool barr[5];
    std::set<uint32_t> myset;
    myset.insert(itr_user->card1);
    myset.insert(itr_user->card2);
    myset.insert(itr_user->card3);
    myset.insert(itr_user->card4);
    myset.insert(itr_user->card5);

    //checksum256 roothash = gethash(from, externalsrc, itr_metadata->trounds);
    checksum256 roothash = gethash(from, externalsrc, 0);
    string rhash;

    for( int i = 0; i < 32; ++i )
    {
        char const byte = roothash.hash[i];
        rhash += hex_chars[ ( byte & 0xF0 ) >> 4 ];
        rhash += hex_chars[ ( byte & 0x0F ) >> 0 ];
    }

    barr[0] = ishold(dump1) == true;
    barr[1] = ishold(dump2) == true;
    barr[2] = ishold(dump3) == true;
    barr[3] = ishold(dump4) == true;
    barr[4] = ishold(dump5) == true;

    // find how many new cards we need to draw
    uint32_t cnt = 0;
    for (int i = 0; i < 5; i++) {
        if (barr[i] != true) {
            cnt++;
        }
    }

    uint32_t newarr[cnt];
    getcards(from, roothash, newarr, cnt, myset, 0, 52);

    cnt = 0;
    if (barr[0] == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card1 = newarr[cnt++];
        });
    }
    if (barr[1] == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card2 = newarr[cnt++];
        });
    }
    if (barr[2] == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card3 = newarr[cnt++];
        });
    }
    if (barr[3] == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card4 = newarr[cnt++];
        });
    }
    if (barr[4] == false) {
        pools.modify(itr_user, _self, [&](auto &p){
            p.card5 = newarr[cnt++];
        });
    }
    pools.modify(itr_user, _self, [&](auto &p){
        p.cardhash2 = string(rhash);
    });


    uint32_t type = checkwin((uint32_t)itr_user->card1, (uint32_t)itr_user->card2, (uint32_t)itr_user->card3,
            (uint32_t)itr_user->card4, (uint32_t)itr_user->card5);
    uint32_t finalratio = ratios[type];

    //jackpot
    uint64_t tempam = 0;
    if (type == 9) {
        auto itr_pevent = pevents.find(0);

        if (itr_user->bet >= 50000) {
            tempam = itr_pevent->eosin * 0.9;
            puevents.emplace(_self, [&](auto &p) {
                p.id = events.available_primary_key();
                p.owner = from;
                p.type = tempam;
            });
        } else if (itr_user->bet >= 10000 && itr_user->bet < 50000) {
            tempam = itr_pevent->eosin * 0.18;
            puevents.emplace(_self, [&](auto &p) {
                p.id = events.available_primary_key();
                p.owner = from;
                p.type = tempam;
            });
        } else if (itr_user->bet >= 2500 && itr_user->bet < 10000) {
            tempam = itr_pevent->eosin * 0.045;
            puevents.emplace(_self, [&](auto &p) {
                p.id = events.available_primary_key();
                p.owner = from;
                p.type = tempam;
            });
        }

        pevents.modify(itr_pevent, _self, [&](auto &p) {
            p.eosin = p.eosin - tempam;
        });
    }


    pools.modify(itr_user, _self, [&](auto &p){
        uint64_t b = p.bet * finalratio + tempam;   //jackpot
        p.betwin = b;
        p.wintype = type;
    });

    // records events if wintype >= straight
    if (itr_user->wintype >= 4) {
        events.emplace(_self, [&](auto &p){
            p.id = events.available_primary_key();
            p.owner = from;
            p.datetime = now();
            p.wintype = itr_user->wintype;
            p.ratio = ratios[itr_user->wintype];
            p.bet = itr_user->bet;
            p.betwin = itr_user->betwin;
            p.card1 = itr_user->card1;
            p.card2 = itr_user->card2;
            p.card3 = itr_user->card3;
            p.card4 = itr_user->card4;
            p.card5 = itr_user->card5;
        });
        eosio_assert(itr_metadata != metadatas.end(), "Metadata is empty.");
        metadatas.modify(itr_metadata, _self, [&](auto &p){
            p.eventcnt = p.eventcnt + 1;
        });
    }
    while (itr_metadata->eventcnt > 32) {
        auto itr_event2 = events.begin();
        events.erase(itr_event2);
        metadatas.modify(itr_metadata, _self, [&](auto &p){
            p.eventcnt = p.eventcnt - 1;
        });
    }

    auto itr_metadatatotal = metadatas.find(0);
    uint64_t mineprice = getminingtableprice(itr_metadatatotal->tmevout);
    uint64_t minemev = itr_user->bet * mineprice * (1 + itr_paccount->level * 0.05)  / 100;
    uint64_t meosin = itr_user->bet;
    uint64_t meosout = itr_user->betwin;

    report(from, minemev, meosin, meosout, 0);
    auto itr_typestat = typestats.find(itr_user->wintype);
    typestats.modify(itr_typestat, _self, [&](auto &p) {
        p.count += 1;
    });

}
*/
bool pokergame1::checkflush(uint32_t colors[5]) {
    uint32_t flag = colors[0];
    for (int i = 1; i < 5; ++i) {
        if (colors[i] != flag) return false;
    }
    return true;
}

bool pokergame1::checkstraight(uint32_t numbers[5]) {
    if (numbers[0] == 0 && numbers[1] == 9 && numbers[2] == 10 && numbers[3] == 11 && numbers[4] == 12) {
        return true;
    }
    for (int i = 1; i < 5; ++i) {
        if (numbers[i] != (numbers[i - 1] + 1)) return false;
    }
    return true;
}

uint32_t pokergame1::checksame(uint32_t numbers[5], uint32_t threshold) {
    uint32_t counts[13];
    for (int i = 0; i < 13; i++) {
        counts[i] = 0;
    }
    for (int i = 0; i < 5; i++) {
        counts[numbers[i]]++;
    }
    uint32_t cnt = 0;
    for (int i = 0; i < 13; i++) {
        if (counts[i] == threshold) cnt++;
    }
    return cnt;
}

// Note: this is only for promo. Check how many A
uint32_t pokergame1::checkace(uint32_t numbers[5]) {
    uint32_t cnt = 0;
    for (int i = 0; i < 5; i++) {
        if (numbers[i] % 13 == 0)
            cnt++;
    }
    return cnt;
}

bool pokergame1::checkBiggerJack(uint32_t numbers[5]) {
    uint32_t counts[13];
    for (int i = 0; i < 13; i++) {
        counts[i] = 0;
    }
    for (int i = 0; i < 5; i++) {
        counts[numbers[i]]++;
    }
    if (counts[0] == 2) return true;
    for (int i = 10; i < 13; i++) {
        if (counts[i] == 2) return true;
    }
    return false;
}

bool pokergame1::ishold(string s) {
    uint32_t pos = s.find(":");
    if (pos > 0) {
        string ucm = s.substr(pos + 1, 4);
        if (ucm == "dump") {
            return false;
        }
    }
    return true;
}

uint32_t pokergame1::parsecard(string s) {
    uint32_t pos = s.find("[");
    if (pos > 0) {
        string ucm = s.substr(0, pos);
        uint32_t res = stoi(ucm);
        return res;
    }
    return 1024;
}


void pokergame1::addpartner(const account_name partner, uint32_t rate) {
    require_auth(N(eosvegasopmk));

    eosio_assert(rate <= 20, "Rate cannot be more than 0.5%!");

    auto itr = partners.find(partner);
    if (itr == partners.end()) {
        partners.emplace(_self, [&](auto &p) {
            p.owner = name{partner};
            p.rate = rate;
            p.count = 0;
            p.eosout = 0;
        });
    } else {
        partners.modify(itr, _self, [&](auto &p) {
            p.rate = rate;
        });
    }
};

void pokergame1::resetdivi() {
    require_auth(_self);
    auto itr = metadatas.find(0);
    metadatas.modify(itr, _self, [&](auto &p) {
        p.teosout = p.teosin;
    });
}


void pokergame1::clear(account_name owner) {
    require_auth(_self);
    print("=======");

    auto itr = metadatas.find(0);
    metadatas.modify(itr, _self, [&](auto &p) {
        p.teosout = p.teosin;
    });

/*
    int cnt = 0;
    auto itr2 = mevouts.begin();
    while (itr2 != mevouts.end() && cnt < 500) {
        auto itr3 = vppools.find(itr2->owner);
        if (itr3 == vppools.end()) {
            itr2 = mevouts.erase(itr2);
        } else {
            itr2++;
        }
        cnt++;
    }*/

    //auto itr = vppools.begin();
    //while (itr != vppools.end()) {
    //    itr = vppools.erase(itr);
    //}

    /*

    auto itr = pools.begin();
    while (itr != pools.end()) {
        itr = pools.erase(itr);
    }

    auto itr = pools.begin();
    while (itr != pools.end()) {
        if (itr->bet == 0) {
            itr = pools.erase(itr);
        } else {
            itr++;
        }
    }

    auto itr2 = events.begin();
    while (itr2 != events.end()) {
        itr2 = events.erase(itr2);
    }
    auto itr3 = metadatas.begin();
    while (itr3 != metadatas.end()) {
        itr3 = metadatas.erase(itr3);
    }

    auto itr5 = ginfos.begin();
    while (itr5 != ginfos.end()) {
        itr5 = ginfos.erase(itr5);
    }
    auto itr6 = gaccounts.begin();
    while (itr6 != gaccounts.end()) {
        itr6 = gaccounts.erase(itr6);
    }
    auto itr7 = cardstats.begin();
    while (itr7 != cardstats.end()) {
        itr7 = cardstats.erase(itr7);
    }
    auto itr8 = typestats.begin();
    while (itr8 != typestats.end()) {
        itr8 = typestats.erase(itr8);
    }
    auto itr9 = paccounts.begin();
    while (itr9 != paccounts.end()) {
        itr9 = paccounts.erase(itr9);
    }


*/
/*
    auto itr10 = pevents.begin();
    while (itr10 != pevents.end()) {
        itr10 = pevents.erase(itr10);
    }

    pevents.emplace(_self, [&](auto &p) {
        p.id = 0;
        p.count = 0;
        p.eosin = 0;
    });
*/
}

void pokergame1::ramclean() {
    require_auth(_self);
/*
    int cnt = 0;
    auto itr = pools.begin();


    while (itr != pools.end() && cnt < 500) {
        if (itr->owner == N(architecture) || (itr->bet == 0 && itr->cardhash1.length() == 0 && itr->cardhash2.length() == 0)) {
            itr = pools.erase(itr);
        } else {
            itr++;
        }
        cnt++;
    }


    int cnt = 0;
    auto itr = gaccounts.find(N(toothree5555));
    while (itr != gaccounts.end() && cnt < 500) {
        if (itr->teosin <= 10) {
            itr = gaccounts.erase(itr);
        } else {
            itr++;
        }
        cnt++;
    }
*/
    //for (int i = 0; i < 1; i++) {
    //    itr++;
    //}
    /*
while (itr != pools.end()) {

    if (itr->cardhash1.length() != 0 && itr->cardhash2.length() != 0) {
        pools.modify(itr, _self, [&](auto &p) {
            p.cardhash1 = "";
            p.cardhash2 = "";
            p.bet = 0;
            p.betwin = 0;
            p.wintype = 0;
            p.card1 = 0;
            p.card2 = 0;
            p.card3 = 0;
            p.card4 = 0;
            p.card5 = 0;
        });

    }
    itr++;
}


uint32_t cnt = 0;
for (auto itr = gaccounts.begin();  cnt < ; itr++) {
    uint32_t lev = getlevel(itr->teosin);

    auto itr_paccount = paccounts.find(itr->owner);
    if (itr_paccount == paccounts.end()) {
        auto itr_paccount = paccounts.emplace(_self, [&](auto &p){
            p.owner = itr->owner;
            p.level = lev;
            p.exp = itr->teosin * 50;
        });
    }

    cnt++;
    */
    /*else {
        paccounts.modify(itr_paccount, _self, [&](auto &p) {
            p.level = lev;
            p.exp = itr->teosin * 50;
        });
    }
}*/



    /*
    auto itr3 = metadatas.begin();
    auto itr_metadata = metadatas.emplace(_self, [&](auto &p){
        p.eventcnt = 0;
        p.idx = 0;
        p.gameon = 1;
        p.miningon = 0;
        p.tmevout = 0;
        p.teosin = 0;
        p.teosout = 0;
        p.trounds = 0;
    });
    auto itr4 = secrets.begin();
    //int bnum = tapos_block_num();
    for( int i = 0; i < 1024; ++i ) {
        secrets.emplace(_self, [&](auto &p) {
            p.id = i;
            p.s1 = current_time() + 7 * i;
        });
    }
    auto itr5 = cardstats.begin();
    for( int i = 0; i < 52; ++i ) {
        cardstats.emplace(_self, [&](auto &p) {
            p.id = i;
            p.count = 0;
        });
    }

    auto itr6 = typestats.begin();
    for( int i = 0; i < 10; ++i ) {
        typestats.emplace(_self, [&](auto &p) {
            p.id = i;
            p.count = 0;
        });
    }
    */
}

void pokergame1::init(string text) {
    require_auth(_self);
}

void pokergame1::setgameon(uint64_t id, uint32_t flag) {
    require_auth(_self);
    auto itr = metadatas.find(id);
    if (itr != metadatas.end()) {
        metadatas.modify(itr, _self, [&](auto &p) {
            p.gameon = flag;
        });
    } else {
        auto itr_metadata = metadatas.emplace(_self, [&](auto &p){
            p.id = id;
            p.eventcnt = 0;
            p.idx = 0;
            p.gameon = flag;
            p.miningon = 0;
            p.tmevout = 0;
            p.teosin = 0;
            p.teosout = 0;
            p.trounds = 0;
        });
    }

}

void pokergame1::blacklist(const name to, uint32_t status) {
    require_auth(_self);


    if (status == 0) {
        auto itr = blacklists.find(to);
        if (itr != blacklists.end()) {
            blacklists.erase(itr);
        }
    } else {
        auto itr = blacklists.find(to);
        if (itr == blacklists.end()) {
            blacklists.emplace(_self, [&](auto &p) {
                p.owner = to;
            });
        }
    }


}

void pokergame1::setminingon(uint64_t id, uint32_t flag) {
    require_auth(_self);
    auto itr = metadatas.find(id);
    if (itr != metadatas.end()) {
        metadatas.modify(itr, _self, [&](auto &p) {
            p.miningon = flag;
        });
    }
}

void pokergame1::setseed(const name from, uint32_t seed) {
    require_auth(_self);

/*
    uint32_t darr[3];
    darr[0] = 3;
    darr[1] = 162;
    darr[2] = 187;
    uint32_t dresult = bj_get_result(darr, 3);

    uint32_t parr[2];
    parr[0] = 141;
    parr[1] = 108;
    uint32_t presult = bj_get_result(parr, 2);

    print("=====", dresult);
    print("=====", presult);
*/
    //require_auth2(from, N(eosio.code));
/*
    auto itr = finaltable.begin();
    if (itr != finaltable.end()) {
        finaltable.modify(itr, _self, [&](auto &p) {
            p.owner = name{a.account};
        });
    } else {
        finaltable.emplace(_self, [&](auto &p) {
            p.owner = name{a.account};
        });
    }



    require_recipient(from );
    */

}


void pokergame1::getbonus(const name from, const uint32_t type, uint32_t externalsrc) {
    require_auth(from);

    sanity_check(N(eosvegasjack), N(getbonus));

    auto itr_pool = pools.find(from);
    auto itr_paccount = paccounts.find(from);
    eosio_assert(itr_pool != pools.end() || itr_paccount != paccounts.end(), "We're under bot attack. Please play once to be whitelisted.");

    uint32_t curtime = now();

    if (itr_paccount == paccounts.end()) {
        itr_paccount = paccounts.emplace(_self, [&](auto &p){
            p.owner = name{from};
            p.level = 0;
            p.exp = 0;
            p.lastbonus = 0;
            p.lastseen = 0;
            p.logins = 0;
            p.bonusnumber = 0;
        });
    }

    eosio_assert(itr_paccount->lastbonus + 8 * 3600 < curtime, "Next bonus is not ready.");

    checksum256 roothash = gethash(from, externalsrc, 0);
    uint64_t temp = roothash.hash[3];
    temp <<= 32;
    temp |= roothash.hash[6];

    uint32_t num = temp % 10000;
    if (num == 9999) num = from % 2 == 0 ? 9997 : 9998;

    paccounts.modify(itr_paccount, _self, [&](auto &p){
        p.lastbonus = curtime;
        p.bonusnumber = num;
    });

    uint64_t bamount = 0;
    if (num <= 5000 ) {
        bamount = 1;
    } else if (num > 5000 && num <= 9885) {
        bamount = 5;
    } else if (num > 9885 && num <= 9935) {
        bamount = 10;
    } else if (num > 9935 && num <= 9985){
        bamount = 50;
    } else if (num > 9985 && num <= 9990) {
        bamount = 100;
    }  else if (num > 9991 && num <= 9996) {
        bamount = 500;
    } else if (num > 9996 && num <= 9998) {
        bamount = 1000;
    } else {
        bamount = 10000;
    }

    asset bal = asset(bamount, symbol_type(S(4, EOS)));
    action(permission_level{_self, N(active)}, N(eosio.token),
        N(transfer), std::make_tuple(_self, from, bal,
                                    std::string("Welcome! Daily bonus to enjoy the game @ rovegas.com!")))
        .send();
}

void pokergame1::forceclear(const name from) {
    require_auth(N(eosvegasjack));

    auto itr = vppools.find(from);
    if (itr != vppools.end()) {
        vppools.erase(itr);
    }
}

void pokergame1::bjclear(const name from) {
    require_auth(N(eosvegasjack));

    auto itr = bjpools.find(from);
    if (itr != bjpools.end()) {
        bjpools.erase(itr);
    }
}

void pokergame1::uthclear(const name from) {
    require_auth(N(eosvegasjack));

    auto itr = uthpools.find(from);
    if (itr != uthpools.end()) {
        uthpools.erase(itr);
    }
}

void pokergame1::pdclear(const name from) {
    require_auth(N(eosvegasjack));

    auto itr = pdpools.find(from);
    if (itr != pdpools.end()) {
        pdpools.erase(itr);
    }
}

void pokergame1::setpubkey(string public_key) {
    require_auth(N(eosvegasjack));

    auto itr = pubkeys.find(0);
    if (itr == pubkeys.end()) {
        pubkeys.emplace(_self, [&](auto &p){
            p.id = pubkeys.available_primary_key();
            p.pubkey = public_key;
            p.updatetime = now();
        });
    } else {
        pubkeys.modify(itr, _self, [&](auto &p){
            p.pubkey = public_key;
            p.updatetime = now();
        });

    }
}

void pokergame1::luck(const name player, uint64_t bonus, uint32_t status) {
    require_auth(N(eosvegasjack));

    auto itr = newyears.find(player);
    eosio_assert(itr != newyears.end(), "2019: user not found");
    eosio_assert(status == 1 || status == 2 || status == 3, "2019: invalid status");

    uint32_t nextstatus = 0;
    if (status == 1) {
        eosio_assert(itr->status == 0, "2019: invalid status");
        nextstatus = 1;
    } else if (status == 2) {
        nextstatus = 2;
        eosio_assert(itr->status == 1, "2019: invalid status");
        eosio_assert(bonus == 200000, "2019: incorrect number");

        asset bal = asset(bonus, symbol_type(S(4, EOS)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosio.token),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Happy New Year! Wish you a wonderful 2019! - www.rovegas.com")))
                    .send();
        }
    } else {
        eosio_assert(itr->status == 2, "2019: invalid status");
        nextstatus = 2019;

        asset bal = asset(bonus, symbol_type(S(4, EOS)));
        if (bal.amount > 0) {
            // withdraw
            action(permission_level{_self, N(active)}, N(eosio.token),
                   N(transfer), std::make_tuple(_self, player, bal,
                                                std::string("Happy New Year! Wish you a wonderful 2019! - www.rovegas.com")))
                    .send();
        }
    }

    newyears.modify(itr, _self, [&](auto &p){
        p.status = nextstatus;
    });
}

#define EOSIO_ABI_EX( TYPE, MEMBERS ) \
extern "C" { \
   void apply(uint64_t receiver, uint64_t code, uint64_t action) { \
      auto self = receiver; \
      if( action == N(onerror)) { \
         /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
         eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
      } \
      if(code == self || code == N(eosio.token) || code == N(eosvegascoin) || code == N(horustokenio) || code == N(everipediaiq)) { \
          if (action == N(transfer) && code == self) { \
              return; \
          } \
          TYPE thiscontract(self); \
          if (action == N(transfer) && code == N(eosio.token)) { \
              currency::transfer tr = unpack_action_data<currency::transfer>(); \
              if (tr.to == self) { \
                  thiscontract.deposit(tr, code, 0); \
              } \
              return; \
          } \
          if (action == N(transfer) && code == N(eosvegascoin)) { \
              currency::transfer tr = unpack_action_data<currency::transfer>(); \
              if (tr.to == self) { \
                  thiscontract.deposit(tr, code, 1); \
              } \
              return; \
          } \
          if (action == N(transfer) && code == N(horustokenio)) { \
              currency::transfer tr = unpack_action_data<currency::transfer>(); \
              if (tr.to == self) { \
                  thiscontract.deposit(tr, code, 2); \
              } \
              return; \
          } \
          if (action == N(transfer) && code == N(everipediaiq)) { \
              currency::transfer tr = unpack_action_data<currency::transfer>(); \
              if (tr.to == self) { \
                  thiscontract.deposit(tr, code, 3); \
              } \
              return; \
          } \
          if (action == N(transfer) && code == N(bitpietokens)) { \
              currency::transfer tr = unpack_action_data<currency::transfer>(); \
              if (tr.to == self) { \
                  thiscontract.deposit(tr, code, 4); \
              } \
              return; \
          } \
          if (code != self) { \
              return; \
          } \
          switch(action) { \
              EOSIO_API(TYPE, MEMBERS) \
          } \
      } \
   } \
}

EOSIO_ABI_EX(pokergame1, (vpreceipt)(vp5xreceipt)(forceclear)(bjclear)(setseed)(setgameon)(setminingon)(signup)(getbonus)(ramclean)(blacklist)(init)(clear)(bjhit)(bjstand)(bjhit1)(bjstand1)(bjhit2)(bjstand2)(bjuninsure)(bjreceipt)(addpartner)(vpdraw)(resetdivi)(setpubkey)(uthfold)(uthcheck)(uthclear)(uthreceipt)(pdclear)(pdreceipt)(luck))