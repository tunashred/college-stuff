#include <crypt.h>
#include <cstring>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

std::string target_salt = "$6$9kfonWC7$";
std::string target_pw_hash = "$6$9kfonWC7$gzqmM9xD7V3zzZDo.3Fb5mAdM0GbIR2DYTtjYpcGkXVWatTC0pa/XVvKTXLb1ZP0NG9cinGRZF7gPLdhJsHDM/";

char null[] = {'\0'};

#define MAX_LEN 8

bool check_password(std::string& pw, char* salt, char* hash) {
    char* res = crypt(pw.c_str(), salt);
    std::cout << "password " << pw << "\n";
    std::cout << "hashes to " << res << "\n";
    if(!strcmp(res, hash)) {
        std::cout << "MATCH" << "\n";
        return 1;
    }
    return 0;
}

char* exhaustive_search(std::vector<std::string> wordset, char* salt, char* target) {
    std::string current_password;
    std::string new_password;
    int i;
    std::list<std::string> pwlist;
    bool pass_found = false;

    do {
        for(auto word : wordset) {
            pwlist.push_back(word);
        }

        while(true) {
            if(pwlist.empty()) {
                break;
            }

            current_password = pwlist.front().c_str();
            pass_found = check_password(current_password, salt, target);
            if(pass_found) {
                char* pass = new char[current_password.length() + 1];
                strcpy(pass, current_password.c_str());
                return pass;
            }

            for(i = 0; i < wordset.size(); i++) {
                if(current_password == wordset[i]) {
                    continue;
                }
                new_password = current_password;
                new_password.append(wordset[i]);
                pwlist.push_back(new_password);
            }
            pwlist.pop_front();
        }
    } while(std::next_permutation(wordset.begin(), wordset.end()));
    return null;
}

int main() {
    char* salt;
    char* target;
    char* password;
    std::vector<std::string> wordset = {"red", "green", "blue", "orange", "pink"};
    std::sort(wordset.begin(), wordset.end());

    salt = new char[target_salt.length() + 1];
    std::copy(target_salt.begin(), target_salt.end(), salt);
    salt[target_salt.length()] = '\0';

    target = new char[target_pw_hash.length() + 1];
    std::copy(target_pw_hash.begin(), target_pw_hash.end(), target);
    target[target_pw_hash.length()] = '\0';

    password = exhaustive_search(wordset, salt, target);
    if(password != null && strlen(password) != 0) {
        std::cout << "Password successfully recovered: " << password << "  \n";
    }
    else {
        std::cout << "Failure to find password, try distinct character set of size \n";
    }

    delete[] salt;
    delete[] target;
    delete[] password;
    return 0;
}
