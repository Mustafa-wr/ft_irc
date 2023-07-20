#include "./includes/Server.hpp"
#include "./includes/Command.hpp"

User::User(int fd, int id) : _fd(fd), _id(id), isAuth(false), isOperator(false), nickName(""), userName("")
{
	std::cout << "User created" << std::endl;
	input = "";
}

User::~User() {}

void User::userErase(User &user)
{
	for(std::vector<User>::iterator it = Server::_users.begin(); it != Server::_users.end(); ++it)
	{
		if (it->_fd == user._fd) {
			Server::_users.erase(it);
			--it;
		}
	}
}

void User::whoAmI(User &user)
{
	std::cout << CYAN << user << RESET <<std::endl;
	std::string userDetails = "UserName: [" + user.userName + "]" + ", Nick: " + "[" + user.nickName + "]" + ", Auth: " 
			+ "[" + (user.isAuth ? "YES" : "NO") + "]" + ", [fd: " +  std::to_string(user._fd) + "]" + ".\n";
	send(user._fd, (YELLOW + userDetails + RESET).c_str(), userDetails.length() + strlen(YELLOW) + strlen(RESET) , 0);
    
}

void User::showClients(User &user)
{
	(void)user;
	for (int i = 0; i < Server::max_sd; i++)
	std::cout << "client " << Server::clientSockets[i] << " " << std::endl;
}

void User::showUsers(User &user)
{
	(void)user;
	Server::showUsers();
}

void closeMe(User &user)
{
	close(user._fd);
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (Server::clientSockets[i] == user._fd)
			Server::clientSockets[i] = 0;
	}
	    
    for(std::vector<int>::iterator it = Server::_fds.begin(); it != Server::_fds.end(); ++it) 
	{
        if (*it == Server::sd) 
            std::cout << "found -->" << *it << std::endl;
            Server::_fds.erase(it);
            --it;
    }
	
    for(std::vector<User>::iterator it = Server::_users.begin(); it != Server::_users.end(); ++it)
	{
        if (it->_fd == Server::sd) {
            std::cout << "id -->" << it->_fd << std::endl;
            Server::_users.erase(it);
            --it;
        }
    }
}

void User::authorise(User *user, std::string cmd)
{
	if (parse_cmds(cmd))
	{
		if (user->isAuth)
		{
			std::string S = ERR_NICKCOLLISION;
			S.append(" User already registered");
			send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
			if(_cmd.size() > 0)
				_cmd.clear();
			return ;
		}
		if(_cmd.size() == NC_LEN)
			user->pass = _cmd[5];
		else if(_cmd.size() == IRSSI_LEN)
			user->pass = _cmd[3];
		if(user->pass != Server::getPassword())
		{
			std::string S = WRONG_PASS_CODE;
			S.append(" Wrong password");
			send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
			if(_cmd.size() > 0)
				_cmd.clear();
			// closeMe(*user);
			return ;
		}
		for(std::vector<User>::iterator it = Server::_users.begin(); it != Server::_users.end(); ++it)
		{
			if (_cmd[1] == it->userName || _cmd[3] == it->nickName)
			{
				std::string S(ERR_NICKCOLLISION);
				S.append(" User exists! : ");
				send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
				if(_cmd.size() > 0)
					_cmd.clear();
				return ;
			}
		}
		if(_cmd.size() == NC_LEN)
		{
			user->nickName = _cmd[3];
			user->userName = _cmd[1];
		}
		else if(_cmd.size() == IRSSI_LEN)
		{
			user->nickName = _cmd[5];
			user->userName = _cmd[7];
		}
		user->isAuth = true;
		send(user->_fd, "Authenticated : ", strlen("Authenticated : "), 0);
		if(_cmd.size() > 0)
			_cmd.clear();
	}
	else
	{
		if(_cmd.size() > 0)
			_cmd.clear();
		// return ;
	}
}

void	User::user_options(User *user, std::vector<std::string> splitmsg)
{
	if (splitmsg.size() > 0 && (splitmsg[0] == "quit" || splitmsg[0] == "exit" || splitmsg[0] == "close"))
		closeMe(*user);
	else if (splitmsg.size() > 0 && splitmsg[0] == "help")
	{
		std::string help = "Available commands: \n"
		"whoami - show user details\n"
		"show clients - show all clients\n"
		"show users - show all users\n"
		"kick <nick> - kick user\n"
		"exit - close connection\n"
		"quit - close connection\n"
		"close - close connection\n"
		"help - show help\n";
		send(user->_fd, help.c_str(), help.length(), 0);
	}
	else
	{
		if(_cmd.size() > 0)
			_cmd.clear();
		// return ;
	}
}

void User::user_cmds(User *user, std::vector<std::string> splitmsg)
{
	Command cmd;
	int i = 1;
	int j;

	if (splitmsg.size() > 0  && splitmsg[0] == "JOIN")
	{
		// if(splitmsg.size() == 3) join with key
		// 	cmd.join(splitmsg[1], splitmsg[2], *user);
		// else
		if(splitmsg.size() == 2 || splitmsg.size() == 3)
			cmd.join(splitmsg[1], "", *user);
		else
		{
			std::string S = TOO_MANY_ARGS;
			S.append(" invalid arguments number");
			send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
			return ;
		}
		// std::cout << "User " << user->nickName << " added to channel " << splitmsg[0] << std::endl; 
	}
	else if ((splitmsg.size() > 0 && splitmsg[0] == "KICK"))
	{
		if(splitmsg.size() == 4)
			cmd.kick(splitmsg[1], splitmsg[2], splitmsg[3], *user);
		else if(splitmsg.size() == 3)
			cmd.kick(splitmsg[1], splitmsg[2], "", *user);
		else
		{
			std::string S = TOO_MANY_ARGS;
			S.append(" invalid arguments number");
			send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
			return ;
		}
	}
	else if ((splitmsg.size() == 2 || splitmsg.size() == 3) && splitmsg[0] == "TOPIC")
	{
		cmd.topic(splitmsg[1], splitmsg[2], *user);
	}
	else if (splitmsg.size() > 0 && splitmsg[0] == "PRIVMSG")
	{
		if(splitmsg.size() == 3)
			cmd.privmsg(splitmsg[1], splitmsg[2], *user);
		if(splitmsg.size() == 2)
		{
			std::string S = PRIVMSG_EMPTY;
			S.append(" No text to send");
			send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
			return ;
		}
		if(splitmsg.size() == 1)
		{
			std::string S = PRIVMSG_NO_USER;
			S.append(" No such user");
			send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
			return ;
		}
		else
		{
			std::string S = TOO_MANY_ARGS;
			S.append(" Too many arguments");
			send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
			return ;
		}
	}
	else if (splitmsg.size() > 0 && splitmsg[0] == "INVITE")
	{
		if(splitmsg.size() == 3)
			cmd.invite(splitmsg[1], splitmsg[2], *user);
		else
		{
			std::string S = TOO_MANY_ARGS;
			S.append(" invalid arguments number");
			send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
			return ;
		}
	}
	for (std::vector<Channel>::iterator it = Server::_channels.begin(); it != Server::_channels.end(); it++)
	{
		std::cout << "Channel " << i << "'s name in server vector -> " << it->getName() << std::endl;
		std::vector<User> temp_users = it->getUsers();
		j = 1;
		for (std::vector<User>::iterator it_u = temp_users.begin(); it_u != temp_users.end(); it_u++)
		{
			std::cout << "User " << j << " - " << it_u->nickName << std::endl;
			j++;
		}
		j = 1;
		for (std::vector<User>::iterator it_i = it->invites.begin(); it_i != it->invites.end(); it_i++)
		{
			std::cout << "Invite " << j << " - " << it_i->nickName << std::endl;
			j++;
		}
		i++;
	}
}

void User::execute(std::string cmd, User *user)
{
	std::string levels[3] = {"whoami", "show clients", "show users"};
	void (User::*f[3])(User &user) = { &User::whoAmI, &User::showClients, &User::showUsers};
	std::vector<std::string> splitmsg = split(cmd);


    if (!parse_cmds(cmd) && user->isAuth == false)
	{
		std::string S = ERR_NOTREGISTERED;
		S.append(" You have not registered");
		send(user->_fd, S.c_str(), strlen(S.c_str()), 0);
		// closeMe();
		// if(_cmd.size() > 0)
		// 	_cmd.clear();
		return ;
	}
	else
	{
		if(_cmd.size() > 0)
			_cmd.clear();
	}
	
	authorise(user, cmd);
	user_options(user, splitmsg);
	user_cmds(user, splitmsg);

	for (int i = 0; i < 3; i++)
	{
		cmd = trim(cmd);	
		cmd == levels[i] ? (this->*f[i])(*user) : (void)0;
	}

	return ;
}


std::ostream& operator<<(std::ostream& out, const User& User)
{
	std::string input = User.input;
	std::string userDetails = "UserName: [" + User.userName + "]" + ", Nick: " + "[" + User.nickName + "]" + ", Auth: " 
		+ "[" + (User.isAuth ? "YES" : "NO") + "]" + ", last inputs: " + "[" + input.erase(input.length() - 1, 1) + "]" + ".";

    out << userDetails;
    return out;
}

bool	User::parse_cmds(std::string str)
{
	std::vector<std::string> vector = split(str);

	if(_cmd.size() > 0)
		_cmd.clear();
	if (vector.size() != NC_LEN && vector.size() != IRSSI_LEN)
	{
		_cmd = vector;
		return false;
	}
	
	if(vector.size() == NC_LEN && (vector[0] != "USER" || vector[2] != "NICK" || \
		vector[4] != "PASS") )
	{
		_cmd = vector;
		return false;
	}
	if(vector.size() == IRSSI_LEN && (vector[0] != "CAP" || vector[1] != "END" || \
		vector[2] != "PASS" || vector[4] != "NICK" || vector[6] != "USER"))
	{
		_cmd = vector;
		return false;
	}
	if(_cmd.size() > 0)
		_cmd.clear();
	_cmd = vector;

	return true;
}