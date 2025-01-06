#include <crypt.h>
#include <cstring>
#include <iostream>
#include <list>
#include <algorithm>

std::string target_salt = "$6$SvT3dVpN$";
std::string target_pw_hash = "$6$SvT3dVpN$lwb3GViLl0J0ntNk5BAWe2WtkbjSBMXtSkDCtZUkVhVPiz5X37WflWL4k3ZUusdoyh7IOUlSXE1jUHxIrg29p.";

char null[] = {'\0'};

#define MAX_LEN 8

std::list<char*> pwlist;

int check_password(char* pw, char* salt, char* hash) {
    char* res = crypt(pw, salt);
    std::cout << "password " << pw << "\n";
    std::cout << "hashes to " << res << "\n";
    if(!strcmp(res, hash)) {
        std::cout << "MATCH" << "\n";
        return 1;
    }
    return 0;
}

bool do_symbol_permutations(char* current_password, char* symbolset, char* salt, char* target) {
    int current_len = strlen(current_password);
    int symbolset_len = strlen(symbolset);

    std::string symbols(symbolset);

    do {
        char* temp_password = new char[current_len + symbols.length() + 1];
        strcpy(temp_password, current_password);
        strcat(temp_password, symbols.c_str());

        if(check_password(temp_password, salt, target)) {
            return true;
        }

        delete[] temp_password;
    } while(std::next_permutation(symbols.begin(), symbols.end()));
    
    return false;
}

char* exhaustive_search(char* charset, char* symbolset, char* salt, char* target) {
    char* current_password;
    char* new_password;
    int i, current_len;

    for(i = 0; i < strlen(charset); i++) {
        new_password = new char[2];
        new_password[0] = charset[i];
        new_password[1] = '\0';
        pwlist.push_back(new_password);
    }

    while(true) {
        if(pwlist.empty()) {
            return null;
        }

        current_password = pwlist.front();
        current_len = strlen(current_password);

        if(current_len + strlen(symbolset) <= MAX_LEN) {
            bool password_found = do_symbol_permutations(current_password, symbolset, salt, target);
            if(password_found) {
                return current_password;
            }
        }

        if(current_len < MAX_LEN - strlen(symbolset)) {
            for(i = 0; i < strlen(charset); i++) {
                if(strchr(current_password, charset[i])) {
                    continue;
                }
                new_password = new char[current_len + 2];
                strcpy(new_password, current_password);
                new_password[current_len] = charset[i];
                new_password[current_len + 1] = '\0';
                pwlist.push_back(new_password);
            }
        }
        pwlist.pop_front();
        delete[] current_password; 
    }
}

int main() {
    char* salt;
    char* target;
    char* password;
    char charset[] = {'a', 'b', 'c', '1', '2', '\0'};
    char symbolset[] = {'!', '#', '@', '\0'};

    salt = new char[target_salt.length() + 1];
    std::copy(target_salt.begin(), target_salt.end(), salt);
    salt[target_salt.length()] = '\0';

    target = new char[target_pw_hash.length() + 1];
    std::copy(target_pw_hash.begin(), target_pw_hash.end(), target);
    target[target_pw_hash.length()] = '\0';

    password = exhaustive_search(charset, symbolset, salt, target);
    if(password != null && strlen(password) != 0) {
        std::cout << "Password successfully recovered: " << password << "  \n";
    }
    else {
        std::cout << "Failure to find password, try distinct character set of size \n";
    }

    delete[] salt;
    delete[] target;
    return 0;
}
