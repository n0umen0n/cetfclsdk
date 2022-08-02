
//ADDS TOKEN TO THE FUND. ALLOCATION 0. SO FUND MANAGERS HAVE TO VOTE TO ACTUALLY INCLUDE THE TOKEN IN THE FUND. ALSO ON_NOTIFY IS REQUIRED FOR EACH ADDED TOKEN.
void addtokuz(vector<double> tokeninfund,
              vector<double> tokenwortheos,
              vector<double> tokenperold,
              vector<double> tokenpercnew,
              vector<int64_t> decimals,
              vector<uint64_t> pairid,
              vector<string> strpairid,
              vector<symbol> token,
              vector<name> contract,
              vector<double> ratio,
              vector<asset> minamount,
              vector<uint64_t> totalvote,
              vector<string> image,
              name community,
              uint64_t pollkey)
{
    require_auth(_self);

    for (size_t i = 0; i < pairid.size(); ++i)

    {
        rebalontb rebaltab(get_self(), _self.value);
        auto existing = rebaltab.find(token[i].code().raw());
         

            if (existing == rebaltab.end() ) 
        {
            rebaltab.emplace(_self, [&](auto& s) {
                s.tokeninfund = tokeninfund[i];
                s.tokenwortheos = tokenwortheos[i];
                s.tokenperold = tokenperold[i];
                s.tokenpercnew = tokenpercnew[i];
                s.decimals = decimals[i];
                s.pairid = pairid[i];
                s.strpairid = strpairid[i];
                s.token = token[i];
                s.contract = contract[i];
                s.ratio = ratio[i];
                s.minamount = minamount[i];
                s.image = image[i];
            });
        }

        if (existing != rebaltab.end() )
              { check(false, "Token already added."); }
    }

    //require_auth ("consortiumtt"_n);

    for (size_t i = 0; i < sym.size(); ++i)

    {
        portftb pollstbl(_self, community.value);

        auto& pede = pollstbl.get(pollkey, "k2ivittupede");

        vector<symbol> homo = pede.answers;

        vector<uint64_t> piider = pede.totalvote;

        homo.push_back(sym[i]);

        piider.push_back(totalvote[i]);

        auto pollsrow = pollstbl.find(pollkey);

        pollstbl.modify(pollsrow, _self, [&](auto& contract) {
            contract.answers = homo;
            contract.totalvote = piider;
        });
    }
}
