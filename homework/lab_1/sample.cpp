#include <crypt.h>
#include <cstring>
#include <iostream>
#include <list>

std::string target_salt = "$6$Iy/hHRfM$";
std::string target_pw_hash = "$6$Iy/hHRfM$gC.Fw7CbqG.Qc9p9X59Tmo5uEHCf0ZAKCsPZuiYUKcejrsGuZtES1VQiusSTen0NRUPYN0v1z76PwX2G2.v1l1";

char null[] = {'\0'};

#define MAX_LEN 6

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

char* exhaustive_search(char* charset, char* salt, char* target) {
    char* current_password;
    char* new_password;
    int i, current_len;

    for (i = 0; i < strlen(charset); i++) {
        new_password = new char[2];
        new_password[0] = charset[i];
        new_password[1] = '\0';
        pwlist.push_back(new_password);
    }

    while (true) {
        if (pwlist.empty()) {
            return null;
        }

        current_password = pwlist.front();
        current_len = strlen(current_password);

        if (check_password(current_password, salt, target)) {
            return current_password;
        }

        if (current_len < MAX_LEN) {
            for (i = 0; i < strlen(charset); i++) {
                new_password = new char[current_len + 2];
                memcpy(new_password, current_password, current_len);
                new_password[current_len] = charset[i];
                new_password[current_len + 1] = '\0';
                pwlist.push_back(new_password);
            }
        }
        pwlist.pop_front();
    }
}

int main() {
    char* salt;
    char* target;
    char* password;
    char charset[] = {'b', 'o', 'g', 'd', 'a', 'n', '\0'};

    salt = new char[target_salt.length() + 1];
    std::copy(target_salt.begin(), target_salt.end(), salt);
    salt[target_salt.length()] = '\0';

    target = new char[target_pw_hash.length() + 1];
    std::copy(target_pw_hash.begin(), target_pw_hash.end(), target);
    target[target_pw_hash.length()] = '\0';

    password = exhaustive_search(charset, salt, target);
    if (strlen(password) != 0) {
        std::cout << "Password  successfuly  recovered:  " << password << "  \n";
    }
    else {
        std::cout << "Failure to find password, try distinct character set of size \n";
    }
                
    return 0;
}