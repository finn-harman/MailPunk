#include "imap.hpp"
#include <string>
#include <cstring>

IMAP::Session::Session(std::function<void()> updateUI) : _updateUI(updateUI) {

  imap = mailimap_new(0,NULL);

}

void IMAP::Session::connect(std::string const& server,  size_t port) {
  
  std::string msg = "Could not connect to server. Error code: ";  
  check_error(mailimap_socket_connect(imap, server.c_str(), port), msg);

}

void IMAP::Session::login(std::string const& userid, std::string const& password) {

  std::string msg = "Could not log in. Error code: ";  
  check_error(mailimap_login(imap, userid.c_str(), password.c_str()), msg); 
      
}

void IMAP::Session::selectMailbox(std::string const& mailbox0) {
  mailbox = mailbox0;  
  
  std::string msg = "Could not select mailbox. Error code: ";
  check_error(mailimap_select(imap, mailbox.c_str()), msg);

}

IMAP::Message** IMAP::Session::getMessages() {
  
  uint32_t mess_count = message_count();

  messages = new Message*[mess_count + 1];

  if (mess_count == 0)
    for (int i = 0 ; i <= 1 ; i++) {
      messages[i] = nullptr;
      return messages;
    }
    
  mailimap_set* set = mailimap_set_new_interval(1, 0);
  mailimap_fetch_type* fetch_type = mailimap_fetch_type_new_fetch_att_list_empty();
  mailimap_fetch_att* fetch_att = mailimap_fetch_att_new_uid();
  clist* fetch_result;
  
  std::string msg_fetch_att = "Could not add information to mailimap_fetch_type structure. Error code: ";
  check_error(mailimap_fetch_type_new_fetch_att_list_add(fetch_type, fetch_att), msg_fetch_att);

  std::string msg_fetch = "Could not fetch. Error code: ";
  check_error(mailimap_fetch(imap, set, fetch_type, &fetch_result), msg_fetch);

  int i = 0;
  clistiter* cur;

  for(cur = clist_begin(fetch_result) ; cur != nullptr ; cur = clist_next(cur)) {
    mailimap_msg_att* msg_att;
    uint32_t uid;

    msg_att = (mailimap_msg_att*)clist_content(cur);
    uid = get_uid(msg_att);

    if (uid) {
      messages[i] = new Message(this, uid);
      i++;
    }
  }
  messages[i] = nullptr;
    
  mailimap_fetch_list_free(fetch_result);
  mailimap_fetch_type_free(fetch_type);
  mailimap_set_free(set);

  return messages;  
}

uint32_t IMAP::Session::message_count() {
  
  mailimap_status_att_list* status_list_attributes;
  status_list_attributes = mailimap_status_att_list_new_empty();

  std::string msg_add_attr = "Could not add status attributes to the list. Error code: ";
  check_error(mailimap_status_att_list_add(status_list_attributes, MAILIMAP_STATUS_ATT_MESSAGES), msg_add_attr);

  mailimap_mailbox_data_status* status_list_values;

  std::string msg_mb_info = "Count not get requested mailbox information. Error code: ";
  check_error(mailimap_status(imap, mailbox.c_str(), status_list_attributes, &status_list_values), msg_mb_info);
  
  auto status_value = ((mailimap_status_info*)clist_content(clist_begin(status_list_values->st_info_list))) -> st_value;

  mailimap_mailbox_data_status_free(status_list_values);
  mailimap_status_att_list_free(status_list_attributes);

  mess_count = status_value;
  return status_value;
}

uint32_t IMAP::Session::_msg_count() {

return mess_count;

}



uint32_t IMAP::Session::get_uid(mailimap_msg_att* msg_att) {
  
  clistiter* cur;
  for (cur = clist_begin(msg_att->att_list) ; cur != nullptr ; cur = clist_next(cur)) {
    mailimap_msg_att_item* item;

    item = (mailimap_msg_att_item*)clist_content(cur);
    if (item->att_type == MAILIMAP_MSG_ATT_ITEM_STATIC)
      if (item->att_data.att_static->att_type == MAILIMAP_MSG_ATT_UID)
	return item-> att_data.att_static->att_data.att_uid;
  }
  return 0;
}
    
IMAP::Message::Message(Session* _session, uint32_t _uid) {

  session = _session;
  uid = _uid;
  
}

IMAP::Session::~Session() {

  for (int i = 0 ; i < mess_count ; i++)
    delete messages[i];
  delete [] messages;
  
  mailimap_logout(imap);
  mailimap_free(imap);
 
}

mailimap* IMAP::Session::_imap() {

  return imap;

}


std::string IMAP::Message::getBody() {
  if (session->_msg_count() == 0)
    return "";
  
  mailimap_set* set = mailimap_set_new_single(uid);
  mailimap_fetch_type* fetch_type = mailimap_fetch_type_new_fetch_att_list_empty();
  mailimap_section* section = mailimap_section_new(nullptr);
  mailimap_fetch_att* fetch_att = mailimap_fetch_att_new_body_peek_section(section);
  clist* fetch_result;
  std::string body;
  
  std::string msg_fetch_att = "Could not add information to mailmap_fetch_type structure. Error code: ";
  check_error(mailimap_fetch_type_new_fetch_att_list_add(fetch_type, fetch_att), msg_fetch_att);

  std::string msg_fetch = "Could not fetch. Error code: ";
  check_error(mailimap_uid_fetch(session->_imap(), set, fetch_type, &fetch_result), msg_fetch);

  clistiter* cur0 = clist_begin(fetch_result);
  mailimap_msg_att* msg_att = (mailimap_msg_att*)clist_content(cur0);

  clistiter* cur;
  for(cur = clist_begin(msg_att->att_list) ; cur != nullptr ; cur = clist_next(cur)) {
    auto item = (mailimap_msg_att_item*)clist_content(cur);

    if (item->att_type == MAILIMAP_MSG_ATT_ITEM_STATIC)
      if (item->att_data.att_static->att_type == MAILIMAP_MSG_ATT_BODY_SECTION)
	if (item->att_data.att_static->att_data.att_body_section->sec_body_part) {
	  body = item->att_data.att_static->att_data.att_body_section->sec_body_part;
	  mailimap_fetch_list_free(fetch_result);
	  mailimap_fetch_type_free(fetch_type);
	  mailimap_set_free(set);
	  return body;
	  }
  }
  mailimap_fetch_list_free(fetch_result);
  mailimap_fetch_type_free(fetch_type);
  mailimap_set_free(set);
  return 0;
}

std::string IMAP::Message::getField(std::string fieldname) {

  if (session->_msg_count() == 0)
    return "";
  
  mailimap_set* set = mailimap_set_new_single(uid);
  mailimap_fetch_type* fetch_type = mailimap_fetch_type_new_fetch_att_list_empty();
  clist* fetch_result;
  std::string field;
  
  clist* hdr_list = clist_new();
  char* field_in = (char*)malloc(100);
  strcpy(field_in, fieldname.c_str());
  clist_append(hdr_list, field_in);
  mailimap_header_list* header_list = mailimap_header_list_new(hdr_list);
  
  mailimap_section* section = mailimap_section_new_header_fields(header_list);
  mailimap_fetch_att* fetch_att = mailimap_fetch_att_new_body_section(section);
  
  std::string msg_fetch_att = "Could not add information to mailmap_fetch_type structure. Error code: ";
  check_error(mailimap_fetch_type_new_fetch_att_list_add(fetch_type, fetch_att), msg_fetch_att);

  std::string msg_fetch = "Could not fetch. Error code: ";
  check_error(mailimap_uid_fetch(session->_imap(), set, fetch_type, &fetch_result), msg_fetch);
  
  clistiter* cur0 = clist_begin(fetch_result);
  mailimap_msg_att* msg_att = (mailimap_msg_att*)clist_content(cur0);

  clistiter* cur;
  for (cur = clist_begin(msg_att->att_list) ; cur != nullptr ; cur = clist_next(cur)) {
    auto item = (mailimap_msg_att_item*)clist_content(cur);

    if (item->att_type == MAILIMAP_MSG_ATT_ITEM_STATIC) {
      if (item->att_data.att_static->att_type == MAILIMAP_MSG_ATT_BODY_SECTION)
	if (item->att_data.att_static->att_data.att_body_section->sec_body_part) {
	  field = item->att_data.att_static->att_data.att_body_section->sec_body_part;
	  mailimap_fetch_list_free(fetch_result);
	  mailimap_fetch_type_free(fetch_type);
	  mailimap_set_free(set);
	  
	  field = field.substr(field.find_first_of(" \t")+1);
	  field.erase(std::remove(field.begin(), field.end(), 13), field.end());
	  field.erase(std::remove(field.begin(), field.end(), 10), field.end());
	  
	  return field;
      }
    }
  } 
  mailimap_fetch_list_free(fetch_result);
  mailimap_fetch_type_free(fetch_type);
  mailimap_set_free(set);

  return field;
}

void IMAP::Message::deleteFromMailbox() {
  mailimap_set* set = mailimap_set_new_single(uid);
  mailimap_flag_list* flag_list = mailimap_flag_list_new_empty();
  mailimap_flag* deleted = mailimap_flag_new_deleted();

  std::string msg_flag_add = "Could not add flag to list of flags. Error code: ";
  check_error(mailimap_flag_list_add(flag_list, deleted), msg_flag_add);

  mailimap_store_att_flags* store_att_flags = mailimap_store_att_flags_new_set_flags(flag_list);

  std::string msg_uid_store = "Could not alter flags for message set. Error code: ";
  check_error(mailimap_uid_store(session->_imap(), set, store_att_flags), msg_uid_store);

  std::string msg_expunge = "Could not permanently remove selected message. Error code: ";
  check_error(mailimap_expunge(session->_imap()), msg_expunge);

  mailimap_store_att_flags_free(store_att_flags);
  mailimap_set_free(set);

  for (int i = 0 ;i < session->_msg_count() ; i++)
    if (session->messages[i]->uid != this->uid)
      delete session->messages[i];
  session->_updateUI();
  delete this;
}
