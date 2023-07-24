#include <iostream>
#include <string>
#include <vector>
#include "includes/Server.hpp"
#include "includes/Command.hpp"

User *Utils::find(int fd) {
	for(std::vector<User>::iterator it = Server::_users.begin(); it != Server::_users.end(); ++it) {
        if (it->_fd == fd) {
            std::cout << "find:: user FD : " << it->_fd << std::endl;
			return &(*it);
        }
    }
	return NULL;
}

void Utils::signalHandler(int signum) {

    std::cout << RED << "Interrupt signal (" << signum << ") received." << RESET << "\n";

    for(std::vector<int>::iterator it = Server::_fds.begin(); it != Server::_fds.end(); ++it) {
        if (*it != 0) {
            std::cout << "Closing socket: " << *it << std::endl;
            close(*it);
        }
    }
    close(Server::serverSocket);
    exit(signum);
}

void Utils::closeThis(User &user)
{
	std::cout << YELLOW << user.input << RESET;
	std::cout << RED << "User " << user._fd << " closed" << RESET << std::endl;
	close(user._fd);
	
    Server::_fds.erase(std::find(Server::_fds.begin(), Server::_fds.end(), user._fd));	
    Server::_users.erase(std::find(Server::_users.begin(), Server::_users.end(), user));
	Server::showUsers();
}


std::string Utils::to_string(int value) {
    std::stringstream ss;
	ss << value;
	std::string s = ss.str();
	return s;
}
std::vector<std::string> Utils::split(const std::string str) {
	std::vector<std::string> vector;
	std::istringstream iss(str);
	std::string cmd;
	while (iss >> std::skipws >> cmd)
		vector.push_back(cmd);
	return vector;
}
std::string Utils::trim(std::string &str) {
    size_t left = str.find_first_not_of('\n');
    size_t right = str.find_last_not_of('\n');
    if (left == std::string::npos || right == std::string::npos)
        return "";
    return str.substr(left, right - left + 1);
}

void sendErrorMessage(int fd, const std::string& message, const std::string& key) {
    std::string errorMsg = key + " ERROR: " + message;
    send(fd, errorMsg.c_str(), strlen(errorMsg.c_str()), 0);
}

void handleJoinCommand(const std::vector<std::string>& splitmsg, Command& cmd, User* user) {
    if (splitmsg.size() == 2 || splitmsg.size() == 3) {
        cmd.join(splitmsg.at(1), "", *user);
    } else {
		return ; // temporary
        // sendErrorMessage(user->_fd, "JOIN command requires 2 or 3 arguments\n", TOO_MANY_ARGS);
    }
}

void handleKickCommand(const std::vector<std::string>& splitmsg, Command& cmd, User* user) {
    if (splitmsg.size() == 4) {
        cmd.kick(splitmsg.at(1), splitmsg.at(2), splitmsg.at(3), *user);
    } else if (splitmsg.size() == 3) {
        cmd.kick(splitmsg.at(1), splitmsg.at(2), "", *user);
    } else {
        sendErrorMessage(user->_fd, "KICK command requires 3 or 4 arguments\n", TOO_MANY_ARGS);
    }
}

void handlePrivMsgCommand(const std::vector<std::string>& splitmsg, Command& cmd, User* user) {
	if (splitmsg.size() == 3) {
		cmd.privmsg(splitmsg.at(1), splitmsg.at(2), *user);
	} else if (splitmsg.size() == 2) {
		sendErrorMessage(user->_fd, "PRIVMSG command requires 3 arguments\n", PRIVMSG_EMPTY);
	} else if(splitmsg.size() == 1) {
		sendErrorMessage(user->_fd, "PRIVMSG command requires 3 arguments\n", ERR_NOSUCHNICK);
	} else {
		sendErrorMessage(user->_fd, "PRIVMSG command requires 3 arguments\n", TOO_MANY_ARGS);
	}
}

void	handleInviteCommand(const std::vector<std::string>& splitmsg, Command& cmd, User* user)
{
	if (splitmsg.size() == 3) {
		cmd.invite(splitmsg.at(1), splitmsg.at(2), *user);
	} else {
		sendErrorMessage(user->_fd, "INVITE command requires 3 arguments\n", TOO_MANY_ARGS);
	}
}

void handleWhoisCommand(const std::vector<std::string>& splitmsg, Command& cmd, User* user) {
	(void)cmd;
	if (splitmsg.size() == 2) {
		std::string nick = "\nname : " + splitmsg.at(1) + "\n";
		send(user->_fd, nick.c_str(), strlen(nick.c_str()), 0);
	} else {
		sendErrorMessage(user->_fd, "WHOIS command requires 2 arguments\n", TOO_MANY_ARGS);
	}
}

void handleModeCommand(const std::vector<std::string>& splitmsg, Command& cmd, User* user)
{
	if (splitmsg.size() == 3){
		cmd.mode(splitmsg.at(1), splitmsg.at(2), *user, splitmsg.at(3));
	} else {
		sendErrorMessage(user->_fd, "MODE command requires 2 arguments\n", TOO_MANY_ARGS);
	}
		
}