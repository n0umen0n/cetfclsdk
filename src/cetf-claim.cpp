void getdiv(name user, name clmspecifier)

{
    require_auth(user);

    //GET WHEN CURRENT PERIOD STARTED AND OTHER STATS
    divperiod_def divpertb(_self, _self.value);
    divperiod divperiter;
    divperiter = divpertb.get();

    divperiodfrq_def divperfqtb(_self, _self.value);
    clmperfreq divperfrqit;
    divperfrqit = divperfqtb.get();

    etffees_def etffeestb(_self, _self.value);
    etffees feeitr;
    feeitr = etffeestb.get();

    feesadjust_def etffeestbadj(_self, _self.value);
    feesadjust feeitradj;
    feeitradj = etffeestbadj.get();

    perzonstkd personstktbl(_self, user.value);
    //CALCULATE HOW MUCH IN TOTAL USER HAS STAKED.
 for (auto iter = personstktbl.begin(); iter != personstktbl.end(); iter++)
{
    //if (iter->staketime + divperfrqit.periodfreq  < current_time_point() && divperiter.claimperiod != iter->stakeperiod) {

    // one period has lapsed
    if (iter->staketime + divperfrqit.periodfreq < current_time_point()) {
        perstotlskd perstottb(_self, _self.value);
        auto totrow = perstottb.find(user.value);

        if (totrow == perstottb.end() )
             
            {
                perstottb.emplace(_self, [&](auto& s) { s.indtotstaked = iter->staked; });
            }
        if (totrow != perstottb.end() )
             
            {
                perstottb.modify(
                    totrow, name("consortiumtt"), [&]( auto& s ) {
                        s.indtotstaked += iter->staked;
                           
                    });
            }
    }
}
//LOOP END CALCULATE HOW MUCH IN TOTAL USER HAS STAKED.

check(clmspecifier == "eosetfeosetf"_n, "Wrong claiming specifier");

claimtimetb claimtab(_self, clmspecifier.value);
auto claimrow = claimtab.find(user.value);

//GET THE PERC BASED ON HOW MUCH STAKED
totstk_def totalstktbl(_self, _self.value);
totstk newstats;

newstats = totalstktbl.get();

//Multiple times declared, should be put on top?
perstotlskd indtotstk(_self, _self.value);
const auto& iter = indtotstk.get(user.value, "Individual has not staked, or stake has not matured.");

check(iter.indtotstaked.amount != 0, "You have nothing staked CETF.");

double percgets = static_cast<double>(iter.indtotstaked.amount) / newstats.totalstaked.amount;

//CHECK IF PERIOD IS STILL ON OR NEW HAS TO START
if (divperiter.periodstart + divperfrqit.periodfreq > current_time_point()) {
    if (claimrow == claimtab.end() )
         
        {
            claimtab.emplace(_self, [&](auto& s) {
                s.claimperiod = divperiter.claimperiod;
                s.user = user;
            });
        }
    //NEW PERIOD NOT STARTED AND USER TRIES TO CLAIM AGAIN.
    if (claimrow != claimtab.end() )
         
        {
            const auto& claimiter = claimtab.get(user.value, "User has not staked nah");

            check(claimiter.claimperiod != divperiter.claimperiod, "New period not started yet.");

            claimtab.modify(
                claimrow, name("consortiumtt"), [&]( auto& s ) {
                    s.claimperiod = divperiter.claimperiod;
                       
                });
        }

    double divsint = (feeitr.totalfees.amount * percgets);

    struct asset divs = {int64_t(divsint), symbol("EOSETF", 4)};

    createetf(user, divs);

    //ADJUSTCRTCLM ADJUSTS TOTAL FEES WHEN NEW PERIOD STARTS.NUMBER IS POSITIVE IF MORE WAS CREATED EOSETF THAT CLAIMED.
    feeitradj.adjustcrtclm.amount -= divs.amount;
    etffeestbadj.set(feeitradj, _self);
}

//TRIGGERING START OF A NEW PERIOD
else {
    divperiter.periodstart = current_time_point();
    divperiter.claimperiod += 1;
    divpertb.set(divperiter, _self);

    if (claimrow == claimtab.end() )
         
        {
            claimtab.emplace(_self, [&](auto& s) {
                s.claimperiod = divperiter.claimperiod;
                s.user = user;
            });
        }
    if (claimrow != claimtab.end() )
         
        {
            claimtab.modify(
                claimrow, name("consortiumtt"), [&]( auto& s ) {
                    s.claimperiod = divperiter.claimperiod;
                       
                });
        }

    //HERE'S THE ADJUSTMENT
    feeitr.totalfees.amount += feeitradj.adjustcrtclm.amount;
    etffeestb.set(feeitr, _self);

    feeitr = etffeestb.get();
    double divsint = (feeitr.totalfees.amount * percgets);
    struct asset divs = {int64_t (divsint), symbol ("EOSETF", 4)};

    createetf(user, divs);

    //ADJUSTCRTCLM ADJUSTS TOTAL FEES WHEN NEW PERIOD STARTS.NUMBER IS POSITIVE IF MORE WAS CREATED EOSETF THAT CLAIMED.
    //feeitradj = etffeestbadj.get();

    /*
feeitr = etffeestb.get();
feeitr.totalfees.amount -= divsint;
etffeestb.set(feeitr, _self);

//PERIOD STARTS WITH 0
feeitradj.adjustcrtclm.amount = 0;
etffeestbadj.set(feeitradj, _self);

*/
    feeitradj.adjustcrtclm.amount = (0 - divsint);
    etffeestbadj.set(feeitradj, _self);
}

feeitr = etffeestb.get();
check(feeitr.totalfees.amount >= 0, "Total fees to be distr fell below 0.");

//THIS IS TABLE THAT TRACKS HOW MUCH INDIVIDUAL HAS STAKED.
auto totalrow = indtotstk.find(user.value);
indtotstk.modify(
    totalrow, name("consortiumtt"), [&]( auto& s ) {
        s.indtotstaked.amount = 0;
           
    });
}

void getcetf(name user, name clmspecifier)

{
    require_auth(user);

    //GET WHEN CURRENT PERIOD STARTED
    divperiod_def divpertb(_self, _self.value);
    divperiod divperiter;
    divperiter = divpertb.get();

    divperiodfrq_def divperfqtb(_self, _self.value);
    clmperfreq divperfrqit;
    divperfrqit = divperfqtb.get();

    perstotstbx perstottb(_self, _self.value);

    indstkdetftb personstktbl(_self, user.value);
    //CALCULATE HOW MUCH IN TOTAL USER HAS STAKED.
 for (auto iter = personstktbl.begin(); iter != personstktbl.end(); iter++)
{
    //if (iter->staketime + divperfrqit.periodfreq  < current_time_point() && divperiter.claimperiod != iter->stakeperiod) {

    if (iter->staketime + divperfrqit.periodfreq < current_time_point()) {
        //perstotstbx perstottb(_self, _self.value);
        auto totrow = perstottb.find(user.value);

        if (totrow == perstottb.end() )
             
            {
                perstottb.emplace(_self, [&](auto& s) {
                    s.indtotstaked = iter->staked;
                    s.staker = user;
                });
            }
        if (totrow != perstottb.end() )
             
            {
                perstottb.modify(
                    totrow, name("consortiumtt"), [&]( auto& s ) {
                        s.indtotstaked += iter->staked;
                           
                    });
            }
    }
}

check(clmspecifier == "cetfcetfcetf"_n, "Wrong claiming specifier");

claimtimetb claimtab(_self, clmspecifier.value);
auto claimrow = claimtab.find(user.value);

//GET THE PERC BASED ON HOW MUCH STAKED
totstk_def totalstktbl(_self, _self.value);
totstk newstats;

newstats = totalstktbl.get();

//Multiple times declared, should be put on top?
//perstotlskd indtotstk(_self, _self.value);
//const auto &iter =indtotstk.get(user.value, "Individual has not staked, or stake has not matured." );

const auto& iter = perstottb.get(user.value, "Individual has not staked, or stake has not matured.");

check(iter.indtotstaked.amount != 0, "You have nothing staked BOXAUJ.");

double percgets = static_cast<double>(iter.indtotstaked.amount) / newstats.totstketf.amount;

//CHECK IF PERIOD IS STILL ON OR NEW HAS TO START
if (divperiter.periodstart + divperfrqit.periodfreq > current_time_point()) {
    if (claimrow == claimtab.end() )
         
        {
            claimtab.emplace(_self, [&](auto& s) {
                s.claimperiod = divperiter.claimperiod;
                s.user = user;
            });
        }
    //NEW PERIOD NOT STARTED AND USER TRIES TO CLAIM AGAIN.
    if (claimrow != claimtab.end() )
         
        {
            const auto& claimiter = claimtab.get(user.value, "User has not staked nah");

            check(claimiter.claimperiod != divperiter.claimperiod, "New period not started yet.");

            claimtab.modify(
                claimrow, name("consortiumtt"), [&]( auto& s ) {
                    s.claimperiod = divperiter.claimperiod;
                       
                });
        }

    //ISSUE CETF

    //const int64_t interval = (2629746);  SIIA MITU SEKUNDIT TAHAME ET YKS INTERVAL KESTAKS
    const int64_t interval = (1200);

    const int64_t secpassed = (300 * divperiter.claimperiod);

    int64_t halvings = (secpassed / interval);

    // x4

    //SIIA IF claimperiod teatud siis lõpeta rewardid.
    check(divperiter.claimperiod <= 1000, "Rewards for liquidity provision have stopped.");

    //int64_t rewardint =  (1250000); //INITIAL SUM TO BE DISTRIBUTED PER PERIOD

    int64_t rewardint = (6666666);  //INITIAL SUM TO BE DISTRIBUTED PER PERIOD

    int64_t divider = pow(2, halvings);

    //int64_t adjrewardint = rewardint/divider * percgets;

    double adjrewardint = rewardint / static_cast<double>(divider) * percgets;

    struct asset cetfreward = {int64_t(adjrewardint * 10000), symbol("CETF", 4)};

    createetf(user, cetfreward);
}

//TRIGGERING START OF A NEW PERIOD
else {
    divperiter.periodstart = current_time_point();
    divperiter.claimperiod += 1;
    divpertb.set(divperiter, _self);

    if (claimrow == claimtab.end() )
         
        {
            claimtab.emplace(_self, [&](auto& s) {
                s.claimperiod = divperiter.claimperiod;
                s.user = user;
            });
        }
    if (claimrow != claimtab.end() )
         
        {
            claimtab.modify(
                claimrow, name("consortiumtt"), [&]( auto& s ) {
                    s.claimperiod = divperiter.claimperiod;
                       
                });
        }

    //ISSUE CETF

    //const int64_t interval = (2629746);  SIIA MITU SEKUNDIT TAHAME ET YKS INTERVAL KESTAKS

    const int64_t interval = (1200);

    const int64_t secpassed = (300 * divperiter.claimperiod);

    int64_t halvings = (secpassed / interval);

    // x4
    check(divperiter.claimperiod <= 100, "Rewards for liquidity provision have stopped.");

    //int64_t rewardint =  (1250000); //INITIAL SUM TO BE DISTRIBUTED PER PERIOD

    int64_t rewardint = (6666666);  //INITIAL SUM TO BE DISTRIBUTED PER PERIOD 524288

    int64_t divider = pow(2, halvings);

    //int64_t adjrewardint = rewardint/divider * percgets;

    double adjrewardint = rewardint / static_cast<double>(divider) * percgets;

    struct asset cetfreward = {int64_t(adjrewardint * 10000), symbol("CETF", 4)};

    createetf(user, cetfreward);
}

auto totalrow = perstottb.find(user.value);
perstottb.modify(
    totalrow, name("consortiumtt"), [&]( auto& s ) {
        s.indtotstaked.amount = 0;
           
    });
}
