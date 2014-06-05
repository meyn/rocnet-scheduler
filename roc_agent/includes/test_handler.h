#ifndef __TEST_HANDLER__H__
#define __TEST_HANDLER__H__
/*
* RoCNet
* Author: Anselm Meyn
*/

typedef struct testItemEntry_t {
  char* key;
  char* cmd;
  int   duration;
  struct testItemEntry_t* next;
} sTestItemEntry;

void   handle_item(char *line);
int    buildTestItemList(char *file);
int    addItem(sTestItemEntry*);
void   printItemsList();
char*  getKeyCommand(const char *keyValue);
char*  exec_cmd(const char *cmdString, char* resultString);
size_t get_response_str(unsigned int respCode, char* respStr);

#endif /* __TEST_HANDLER__H__ */
