/*
 * EFUpdate.h
 *
 * Project: JurasicParkGate - 
 * Copyright (c) 2023 Martin Mueller
 * http://www.MartnMueller2003.com
 *
 *  This program is provided free for you to use in any way that you wish,
 *  subject to the laws and regulations where you are using it.  Due diligence
 *  is strongly suggested before using this code.  Please give credit where due.
 *
 *  The Author makes no warranty of any kind, express or implied, with regard
 *  to this program or the documentation contained in this document.  The
 *  Author shall not be liable in any event for incidental or consequential
 *  damages in connection with, or arising out of, the furnishing, performance
 *  or use of these programs.
 *
 */

#ifndef EFUPDATE_H_
#define EFUPDATE_H_

#define EFUPDATE_ERROR_OK   (0)
#define EFUPDATE_ERROR_SIG  (100)
#define EFUPDATE_ERROR_REC  (101)

class EFUpdate{
public:
EFUpdate ()
{}
virtual~EFUpdate ()
{}
const uint32_t EFU_ID = 0x00554645;      // 'E', 'F', 'U', 0x00

void begin ();
bool process (uint8_t*  data,
 uint32_t               len);
bool hasError ();
uint8_t getError ();
bool end ();
void GetDriverName (String & name)
{
    name = String ( F ("EFUPD") );
}
private:
/* Record types */
enum class RecordType : uint16_t
{
    NULL_RECORD,
    SKETCH_IMAGE,
    FS_IMAGE,
    EEPROM_IMAGE
};

/* Update State */
enum class State : uint8_t
{
    HEADER,
    RECORD,
    DATA,
    FAIL
};

/* EFU Header */
typedef union
{
    struct
    {
        uint32_t signature;
        uint16_t version;
    } __attribute__ ( (packed) );

    uint8_t raw[6];
} efuheader_t;

/* EFU Record */
typedef union
{
    struct
    {
        RecordType type;
        uint32_t size;
    } __attribute__ ( (packed) );

    uint8_t raw[6];
} efurecord_t;

State _state = State::FAIL;
uint32_t _loc   = 0;
efuheader_t _header;
efurecord_t _record;
uint32_t _maxSketchSpace;
uint8_t _error;
}; // class EFUpdate

#endif /* EFUPDATE_H_ */
