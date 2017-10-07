#ifndef _RF_TYPES_H
#define _RF_TYPES_H

// data_custom is a spare encoding and could easily be used to send your own data type
enum data_type : uint8_t {
  data_byte = 1,
  data_int = 2,
  data_long = 3,
  data_custom = 0
};

// t_custom is a spare reliability type and could easily implement your own
enum reliability_mode : uint8_t {
  t_unsafe = 1,
  t_safe = 2,
  t_custom_2 = 3,
  t_custom_1 = 0
};

#endif