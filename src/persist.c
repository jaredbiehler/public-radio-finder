#include "network.h"
#include "persist.h"

void load_persisted_values(NprData *npr_data) 
{
  if (persist_exists(KEY_ZIP_CODE)) {
    persist_read_string(KEY_ZIP_CODE, npr_data->zip_code, sizeof(npr_data->zip_code));
  } else {
    strncpy(npr_data->zip_code, "", 6);
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "PersistLoad:  zip:%s", npr_data->zip_code);
}

void store_persisted_values(NprData *npr_data) 
{
  persist_write_string(KEY_ZIP_CODE, npr_data->zip_code);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "PersistStore:  zip:%s", npr_data->zip_code);
}