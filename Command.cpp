/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: arafeeq <arafeeq@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 22:38:24 by arafeeq           #+#    #+#             */
/*   Updated: 2023/07/23 16:33:33 by arafeeq          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "./includes/Server.hpp"
#include "./includes/Command.hpp"
#include "./includes/User.hpp"

Command::Command(void)
{
	// defualt constructor
	this->arg_amt = 0;
	this->command = "";
	this->message = "";
	
}

Command::~Command(void)
{
	// destructor
}

// - MEMBER FUNCTIONS -

std::vector<std::string> Command::ft_split(std::string str, char delimiter)
{
	std::vector<std::string> substrings;
	std::string substring = "";
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] != delimiter)
		{
			substring += str[i];
		}
		else
		{
			substrings.push_back(substring);
			while (str[i] == delimiter)
				i++;
			i--;
			substring = "";
		}
	}
	substrings.push_back(substring);
	return (substrings);
}

void Command::join(std::string channel_s, std::string key_s, User user)
{
	//  ------  have to put conditions for invite only channel -------
	std::vector<std::string> channel_split = ft_split(channel_s, ',');
	std::vector<std::string> key_split = ft_split(key_s, ',');
	std::vector<Channel>::iterator it;
	std::vector<std::string>::iterator it_s;
	std::vector<User>::iterator it_u;
	std::vector<std::string>::iterator it_k = key_split.begin();
	
	// check if channel exists
	for (it_s = channel_split.begin(); it_s != channel_split.end(); it_s++)
	{
		for (it = Server::_channels.begin(); it != Server::_channels.end(); it++)
		{
			if (*it_s == it->getName())
			{
				// check if user exists already
				std::vector<User> temp_users = it->getUsers();
				for (it_u = temp_users.begin(); it_u != temp_users.end(); it_u++)
				{
					if (it_u->nickName == user.nickName)
					{
						// should send tho the client or server??
						std::cout << RED_LIGHT << "Error: User already exists in the channel" << RESET << std::endl;
						return ;
					}
				}
				if (it_k != key_split.end())
				{
					if (it->isMode('k') == 1)
					{
						if (*it_k == it->getPass())
						{
							if (it->isMode('i') == 1)
							{
								if (it->isInvited(user))
									it->addUser(user);
								else
									sendErrorMessage(user._fd, (channel_s + NO_INV_M), ERR_INVITEONLYCHAN);
							}
							else
								it->addUser(user);
						}
						else
						sendErrorMessage(user._fd, (channel_s + NO_KEY_M), ERR_BADCHANNELKEY);
					}
					else
						sendErrorMessage(user._fd, "Key Not required to join channel\n", ERR_BADCHANNELKEY);
						
					it_k++;
				}
				else
				{
					if (it->isMode('i') == 1)
					{
						if (it->isInvited(user))
						{
							if (it->isMode('k') == 1)
								sendErrorMessage(user._fd, (channel_s + NO_KEY_M), ERR_BADCHANNELKEY);
							else
								it->addUser(user);
						}
						else
							sendErrorMessage(user._fd, (channel_s + NO_INV_M), ERR_INVITEONLYCHAN);
					}
					else
						it->addUser(user);
				}
				break ;
			}
		}
		// if channel doesnot exist
		if (it == Server::_channels.end())
		{
			if (it_k != key_split.end())
			{
				Channel new_channel(*it_s, *it_k);
				new_channel.addUser(user);
				Server::_channels.push_back(new_channel);
				it_k++;
			}
			else
			{
				Channel new_channel(*it_s, "");
				new_channel.addUser(user);
				Server::_channels.push_back(new_channel);
			}
			std::cout << "Channel " << *it_s << " created!" << std::endl;
		}
	}
}

void Command::kick(std::string channel, std::string user_kick, std::string reason, User user)
{
	std::vector<Channel>::iterator it_c;
	

	for(it_c = Server::_channels.begin(); it_c != Server::_channels.end(); ++it_c)
	{
		if (it_c->getName() == channel)
			break ;
	}
	if (it_c == Server::_channels.end())
		sendErrorMessage(user._fd, (user.nickName + NO_CHAN_M), ERR_NOSUCHCHANNEL);
	else
		it_c->kickUser(user_kick, reason, user);
}

void Command::invite(std::string user, std::string channel, User user_o)
{
	// ERR_USERONCHANNEL - implement
	std::vector<Channel>::iterator it_c;
	std::vector<User>::iterator it_s;

	for(it_c = Server::_channels.begin(); it_c != Server::_channels.end(); ++it_c)
	{
		if (it_c->getName() == channel)
			break ;
	}
	if (it_c == Server::_channels.end())
		sendErrorMessage(user_o._fd, (channel + NO_CHAN_M), ERR_NOSUCHCHANNEL);
	for(it_s = Server::_users.begin(); it_s != Server::_users.end(); ++it_s)
	{
		if (it_s->nickName == user)
		{
			if (it_c->isOperator(user_o) != 1)
				sendErrorMessage(user_o._fd, OP_ERR_M, ERR_CHANOPRIVSNEEDED);
			else
			{
				if (it_c->isUser(*it_s))
					sendErrorMessage(user_o._fd, (user + " " + channel + YES_USR_M), ERR_USERONCHANNEL);
				else
				{
					message = "You're invited to the Channel " + channel + " \n";
					send(it_s->_fd, message.c_str(), strlen(message.c_str()), 0);
					it_c->invites.push_back(*it_s);
				}
			}
			
		}
	}
	if (it_s == Server::_users.end())
		sendErrorMessage(user_o._fd, (user + NO_USR_M), ERR_NOSUCHNICK);
	
}

void Command::topic(std::string channel, std::string topic, User user)
{
	std::vector<Channel>::iterator it_c;

	for(it_c = Server::_channels.begin(); it_c != Server::_channels.end(); ++it_c)
	{
		if (it_c->getName() == channel)
			break ;
	}
	if (it_c != Server::_channels.end())
	{
		if (it_c->isMode('t') == 1)
		{
			if (it_c->isOperator(user) == 1)
			{
				if (topic != "")
				{
					it_c->setTopic(topic);
					message = "Topic of channel " + channel + "changed." + "\n";
					send(user._fd, message.c_str(), strlen(message.c_str()), 0);
				}
				else
				{
					message = "Channel " + channel + "'s topic: " + it_c->getTopic() + "\n";
					send(user._fd, message.c_str(), strlen(message.c_str()), 0);
				}
			}
			else
				sendErrorMessage(user._fd, OP_ERR_M, ERR_CHANOPRIVSNEEDED);
		}
		else
		{
			if (topic != "")
			{
				it_c->setTopic(topic);
				message = "Topic of channel " + channel + "changed." + "\n";
				send(user._fd, message.c_str(), strlen(message.c_str()), 0);
			}
			else
			{
				message = "Channel " + channel + "'s topic: " + it_c->getTopic() + "\n";
				send(user._fd, message.c_str(), strlen(message.c_str()), 0);
			}
		}
	}
	else
	{
		sendErrorMessage(user._fd, (channel + NO_CHAN_M), ERR_NOSUCHCHANNEL);
	}
}

void Command::privmsg(std::string reciever, std::string message, User user)
{
	std::vector<Channel>::iterator it_c;
	std::vector<User>::iterator it_u;

	for(it_u = Server::_users.begin(); it_u != Server::_users.end(); ++it_u)
	{
		if (it_u->nickName == reciever)
			break ;
	}
	if (it_u == Server::_users.end())
	{
		for(it_c = Server::_channels.begin(); it_c != Server::_channels.end(); ++it_c)
		{
			if (it_c->getName() == reciever)
				break ;
		}
		if (it_c != Server::_channels.end())
		{
			if (it_c->isUser(user))
			{
				std::vector<User> temp_users = it_c->getUsers();
				for(std::vector<User>::iterator it = temp_users.begin(); it != temp_users.end(); ++it)
				{
					if(it->_fd == user._fd)
					send((*it)._fd, (message + "\n").c_str(), strlen((message + "\n").c_str()), 0);
				}
			}
			else
				sendErrorMessage(user._fd, (it_c->getName() + NOT_CHAN_USR), ERR_CANNOTSENDTOCHAN);
		}
	}
	else // the receiver and the message
	{
		// any message to server??
		if(user._fd == it_u->_fd)
		{
			send(it_u->_fd, "can't send message to same user\n", strlen("can't send message to same user\n"), 0);
			return ;
		}
		send(it_u->_fd, (message + "\n").c_str(), strlen((message + "\n").c_str()), 0);
	}
	if (it_u == Server::_users.end() && it_c == Server::_channels.end())
		sendErrorMessage(user._fd, (reciever + NO_USR_M + " or channel."), ERR_NOSUCHNICK);
}

void Command::mode(std::string channel, std::string mode, User user, std::string key)
{
	if (mode.size() != 2 && (mode[0] != '+' && mode[0] != '-'))
	{
		sendErrorMessage(user._fd, (mode + MODE_ERR_M), ERR_UNKNOWNMODE);
		return ;
	}
	std::vector<Channel>::iterator it_c;

	for(it_c = Server::_channels.begin(); it_c != Server::_channels.end(); ++it_c)
	{
		if (it_c->getName() == channel)
			break ;
	}
	if (it_c == Server::_channels.end())
		sendErrorMessage(user._fd, (channel + NO_CHAN_M), ERR_NOSUCHCHANNEL);
	else
	{
		if (it_c->isOperator(user))
		{
			if (it_c->isMode(mode[1]) == 2)
				sendErrorMessage(user._fd, (mode + MODE_ERR_M), ERR_UNKNOWNMODE);
			else
			{
				if (mode == "+k" && key == "")
					sendErrorMessage(user._fd, "Key for Channel not provided\n", TOO_MANY_ARGS);
				else
					it_c->setMode(mode[1], mode[0], key);
			}
		}
		else
			sendErrorMessage(user._fd, OP_ERR_M, ERR_CHANOPRIVSNEEDED);
	}
}

// - GETTERS -

User* Command::getSender(void)
{
	return (sender);
}

int Command::getArgAmt(void)
{
	return (arg_amt);
}

std::string Command::getCommand(void)
{
	return (command);
}

std::vector<std::string> Command::getArgs(void)
{
	return (args);
}

// - SETTERS -

void Command::setSender(User* s)
{
	sender = s; // correct??
}

void Command::setArgAmt(int a)
{
	arg_amt = a;
}

void Command::setCommand(std::string c)
{
	command = c;
}

void Command::setArgs(std::vector<std::string> ar)
{
	// clear first?
	args = ar;
}