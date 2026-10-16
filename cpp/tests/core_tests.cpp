#include "RepairCore.hpp"
#include <iostream>
#include <stdexcept>
void require(bool condition,const char *message) {if(!condition)throw std::runtime_error(message);}
int main() {
 try {const auto brands=mr::catalog();require(brands.size()>=10,"catalog missing");require(brands.front().brand=="Samsung","catalog mismatch");
 for(const auto &s:{"Bootloop","Not charging","No network","USB not detected","unknown"}) {require(!mr::diagnose(s).nextCheck.empty(),"missing diagnosis");require(!mr::diagnose(s).safety.empty(),"missing safety");}
 require(mr::validCase("Samsung","A52","Bootloop"),"valid input rejected");require(!mr::validCase(" \t\n","A52","Bootloop"),"empty accepted");require(!mr::validCase("Samsung","","Bootloop"),"empty model accepted");
 std::cout<<"Core rules and input validation passed\n";return 0;
 } catch(const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
}
