#include "stdafx.h"

#include "Clients.h"

void clientLogin(Data *data, Message message) {
  debug(TEXT("Login : %s %d"), message.text, message.number);
}