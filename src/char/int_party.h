// $Id$
#ifndef _INT_PARTY_H_
#define _INT_PARTY_H_

void inter_party_init();
void inter_party_save();

int inter_party_parse_frommap(int fd);

void inter_party_leave(int party_id, int account_id);

#ifdef TXT_ONLY
extern char party_txt[1024];
#endif /* TXT_ONLY */

#endif // _INT_PARTY_H_
