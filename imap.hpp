#ifndef IMAP_H
#define IMAP_H
#include "imaputils.hpp"
#include <libetpan/libetpan.h>
#include <string>
#include <functional>

namespace IMAP {

class Message;
  
class Session {
  
        uint32_t  message_count();
        std::string mailbox;
        uint32_t get_uid(mailimap_msg_att* msg_att);
        uint32_t mess_count;
        mailimap* imap;
public:
        Message** messages;

        uint32_t _msg_count();
        mailimap* _imap();
  
        std::function<void()> _updateUI;
  
	Session(std::function<void()> updateUI);

	/**
	 * Get all messages in the INBOX mailbox terminated by a nullptr (like we did in class)
	 */
	Message** getMessages();

	/**
	 * connect to the specified server (143 is the standard unencrypted imap port)
	 */
	void connect(std::string const& server, size_t port = 143);

	/**
	 * log in to the server (connect first, then log in)
	 */
	void login(std::string const& userid, std::string const& password);

	/**
	 * select a mailbox (only one can be selected at any given time)
	 * 
	 * this can only be performed after login
	 */
        void selectMailbox(std::string const& mailbox0);

	~Session();
};

class Message {

        Session* session;
        uint32_t uid;
public:

        /**
	 * Construct Message with session and uid
	 */ 
        Message(Session* _session, uint32_t _uid);
	/**
	 * Get the body of the message. You may chose to either include the headers or not.
	 */
	std::string getBody();
	/**
	 * Get one of the descriptor fields (subject, from, ...)
	 */
	std::string getField(std::string fieldname);
	/**
	 * Remove this mail from its mailbox
	 */
	void deleteFromMailbox();

};
}

#endif /* IMAP_H */
