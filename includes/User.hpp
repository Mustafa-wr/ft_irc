#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>

class User {
   public:
    User(int fd, int id);
    ~User();

    std::string input;

    int _fd;
    int _id;
    bool isAuth;
    std::string nickName;
    std::string userName;
    std::string realName;
	std::string pass;
    std::vector<std::string> _cmd;

    void execute(std::string cmd, User *it);
    void userErase(User &user);
    bool	parse_cmds(std::string str);
    friend std::ostream& operator<<(std::ostream& os, const User& user);

};

std::ostream& operator<<(std::ostream& out, const User& User);
std::vector<std::string> split(std::string str);
#endif // User_HPP