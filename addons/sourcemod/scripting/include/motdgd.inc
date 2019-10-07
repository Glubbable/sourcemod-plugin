#if defined _motdgd_included_
  #endinput
#endif
#define _motdgd_included_

native bool MOTDgd_AddRewardHandler(char id[32], Function callback);
native bool MOTDgd_RemoveRewardHandler(char id[32]);