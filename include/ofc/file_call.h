/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_FILE_CALL_H__)
#define __OFC_FILE_CALL_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/handle.h"
#include "ofc/file.h"

enum {
    FILE_CALL_TAG_CREATE,
    FILE_CALL_TAG_CLOSE,
    FILE_CALL_TAG_DELETE,
    FILE_CALL_TAG_FIND_FIRST,
    FILE_CALL_TAG_FIND_NEXT,
    FILE_CALL_TAG_FIND_CLOSE,
    FILE_CALL_TAG_FLUSH_BUFFERS,
    FILE_CALL_TAG_GET_FILEEX,
    FILE_CALL_TAG_QUERY_INFO,
    FILE_CALL_TAG_MOVE_FILE,
    FILE_CALL_TAG_READ,
    FILE_CALL_TAG_WRITE,
    FILE_CALL_TAG_SET_EOF,
    FILE_CALL_TAG_SET_FILEEX,
    FILE_CALL_TAG_SET_HFILEEX,
    FILE_CALL_TAG_SET_FILE_POINTER,
    FILE_CALL_TAG_TRANSACT_LANMAN,
    FILE_CALL_TAG_TRANSACT2_NAMED_PIPE,
    FILE_CALL_TAG_GET_VOLUME_INFO,
    FILE_CALL_TAG_REMOVE_DIR,
    FILE_CALL_TAG_LOCK,
    FILE_CALL_TAG_NOTIFY_COMPLETE,
    FILE_CALL_TAG_DISMOUNT,
    FILE_CALL_TAG_DEVICE_IO_CONTROL,
    FILE_CALL_TAG_QUERY_DIRECTORY,
};

/*
 * Overlapped I/O Complete Notification
 */
typedef struct {
    OFC_DWORD nNumberOfBytesTransferred; /**< The number of bytes xferred */
    OFC_UINT32 offset;
} FILE_NOTIFY_COMPLETE;

/*
 * Security attributes descriptor is pushed
 */
typedef struct {
    OFC_DWORD dwDesiredAccess;
    OFC_DWORD dwShareMode;
    OFC_DWORD secLength;
    OFC_BOOL secInherit;
    OFC_DWORD dwCreationDisposition;
    OFC_DWORD dwFlagsAndAttributes;
    OFC_DWORD dwCreateOptions;
    OFC_UINT32 createAction;
    OFC_UINT64 CreationTime;
    OFC_UINT64 LastAccessTime;
    OFC_UINT64 LastWriteTime;
    OFC_UINT64 LastChangeTime;
    OFC_UINT32 dwExtAttributes;
    OFC_UINT64 AllocationSize;
    OFC_UINT64 EndOfFile;
    OFC_UINT16 fileType;
    OFC_UINT16 deviceState;
    OFC_UINT16 Directory;
} FILE_CALL_CREATE;

typedef struct {
    OFC_HANDLE find_file;
    OFC_UINT64 CreationTime;
    OFC_UINT64 LastAccessTime;
    OFC_UINT64 LastWriteTime;
    OFC_UINT64 LastChangeTime;
    OFC_UINT32 FileAttributes;
    OFC_UINT64 AllocationSize;
    OFC_UINT64 EndOfFile;
} FILE_CALL_CLOSE;

typedef struct {
    OFC_INT empty;
} FILE_CALL_DELETE;

typedef struct {
    OFC_INT empty;
} FILE_CALL_REMOVE_DIR;

/* Find data is pushed */
typedef struct {
    OFC_BOOL more;
    OFC_BOOL close_after;
    OFC_BOOL close_eos;
    OFC_UINT16 search_count;
    OFC_SIZET nFindFileDataSize;
} FILE_CALL_FIND;

/* Query Directory data is pushed */
typedef struct {
    OFC_BOOL more;
    OFC_UINT32 name_length;
    OFC_UINT32 buffer_length;
    OFC_HANDLE find_file;
    OFC_BOOL reopen;
} FILE_CALL_QUERY_DIRECTORY;

typedef struct {
    OFC_INT empty;
} FILE_CALL_FIND_CLOSE;


typedef struct {
    OFC_INT empty;
} FILE_CALL_FLUSH_BUFFERS;

/* lpFileInformation is pushed */
typedef struct {
    OFC_FILE_INFO_BY_HANDLE_CLASS FileInformationClass;
    OFC_DWORD dwBufferSize;
} FILE_CALL_GET_FILEEX;

/* lpFileInformation is pushed */
typedef struct {
    OFC_FILE_INFO_BY_HANDLE_TYPE FileInformationType;
    OFC_DWORD FileInformationClass;
    OFC_DWORD dwBufferSize;
} FILE_CALL_QUERY_INFO;

/* new file name is pushed for move */

typedef struct {
    OFC_LPVOID lpBuffer;
    OFC_DWORD nNumberOfBytesToRead;
    OFC_DWORD nNumberOfBytesRead;
    OFC_HANDLE hOverlapped;
    /*
     * These are added to help the CIFS write call.  Others that call write
     * should set distance to move to 0 and move method to FILE_CURRENT.
     */
    OFC_LONG lDistanceToMove;
    OFC_LONG lDistanceToMoveHigh;
    OFC_DWORD dwMoveMethod;
    OFC_DWORD nLowOffset;
    OFC_DWORD nHighOffset;
} FILE_CALL_READ;

typedef struct {
    OFC_LPCVOID lpBuffer;
    OFC_DWORD nNumberOfBytesToWrite;
    OFC_DWORD nNumberOfBytesWritten;
    OFC_HANDLE hOverlapped;
    /*
     * These are added to help the CIFS write call.  Others that call write
     * should set distance to move to 0 and move method to FILE_CURRENT.
     */
    OFC_LONG lDistanceToMove;
    OFC_LONG lDistanceToMoveHigh;
    OFC_DWORD dwMoveMethod;
    OFC_DWORD nLowOffset;
    OFC_DWORD nHighOffset;
} FILE_CALL_WRITE;

typedef struct {
    OFC_INT empty;
} FILE_CALL_SET_EOF;

/* lpFileInformation is pushed */
typedef struct {
    OFC_FILE_INFO_BY_HANDLE_CLASS FileInformationClass;
    OFC_DWORD dwBufferSize;
} FILE_CALL_SET_FILEEX;

/* lpFileInformation is pushed */
typedef struct {
    OFC_FILE_INFO_BY_HANDLE_CLASS FileInformationClass;
    OFC_DWORD dwBufferSize;
} FILE_CALL_SET_HFILEEX;

typedef struct {
    OFC_LONG lDistanceToMove;
    OFC_LONG lDistanceToMoveHigh;
    OFC_DWORD dwMoveMethod;
    OFC_UINT64 dwPosition;
} FILE_CALL_SET_FILE_POINTER;

typedef struct {
    OFC_DWORD nInBufferSize;
    OFC_DWORD nOutBufferSize;
    OFC_DWORD nBytesRead;
    OFC_HANDLE hOverlapped;
} FILE_CALL_TRANSACT2_NAMED_PIPE;

typedef struct {
    OFC_DWORD dwIoControlCode;
    OFC_DWORD nInBufferSize;
    OFC_DWORD nOutBufferSize;
    OFC_DWORD nBytesReturned;
    OFC_HANDLE waitqOverlapped;
} FILE_CALL_DEVICE_IO_CONTROL;

typedef struct {
    OFC_DWORD nInParamSize;
    OFC_DWORD nInDataSize;
    OFC_DWORD nOutParamSize;
    OFC_DWORD nOutParamRead;
    OFC_DWORD nOutDataSize;
    OFC_DWORD nOutDataRead;
} FILE_CALL_TRANSACT_LANMAN;

/* volume name buffer is pushed */
typedef struct {
    OFC_DWORD dwSectorsPerCluster;
    OFC_DWORD dwBytesPerSector;
    OFC_DWORD dwNumberOfFreeClusters;
    OFC_DWORD dwTotalNumberOfClusters;
    OFC_DWORD nVolumeNameSize;
    OFC_DWORD dwVolumeSerialNumber;
    OFC_DWORD dwMaximumComponentLength;
    OFC_DWORD dwFileSystemFlags;
    OFC_DWORD nFileSystemNameSize;
} FILE_CALL_GET_VOLUME_INFO;

typedef struct {
    OFC_UINT32 offset_high;
    OFC_UINT32 offset_low;
    OFC_UINT32 length_high;
    OFC_UINT32 length_low;
} FILE_CALL_LOCK_RANGE;

typedef enum {
    FileCallLockLevelShared,
    FileCallLockLevelExclusive
} FILE_CALL_LOCK_LEVEL;

/* ranges are pushed */
typedef struct {
    FILE_CALL_LOCK_LEVEL lock_level;
    OFC_BOOL change_level;
    OFC_UINT16 num_unlocks;
    OFC_UINT16 num_locks;
} FILE_CALL_LOCK;

typedef struct {
    OFC_INT empty;
} FILE_CALL_DISMOUNT;

/* file name is pushed */
typedef struct {
    OFC_BOOL status;
    OFC_DWORD dwLastError;
    OFC_HANDLE hFile;
    OFC_MSTIME stamp;
    OFC_VOID *context;

    OFC_HANDLE response_queue;
    OFC_UINT command;
    union {
        FILE_CALL_CREATE create;
        FILE_CALL_CLOSE close;
        FILE_CALL_DELETE del;
        FILE_CALL_FIND find;
        FILE_CALL_FIND_CLOSE find_close;
        FILE_CALL_FLUSH_BUFFERS flush_buffers;
        FILE_CALL_GET_FILEEX get_fileex;
        FILE_CALL_QUERY_INFO query_info;
        FILE_CALL_READ read;
        FILE_CALL_WRITE write;
        FILE_CALL_SET_EOF set_eof;
        FILE_CALL_SET_FILEEX set_fileex;
        FILE_CALL_SET_HFILEEX set_hfileex;
        FILE_CALL_SET_FILE_POINTER set_file_pointer;
        FILE_CALL_TRANSACT2_NAMED_PIPE transact2_named_pipe;
        FILE_CALL_TRANSACT_LANMAN transact_lanman;
        FILE_CALL_GET_VOLUME_INFO get_volume_info;
        FILE_CALL_REMOVE_DIR remove_dir;
        FILE_CALL_LOCK lock;
        FILE_NOTIFY_COMPLETE complete;
        FILE_CALL_DISMOUNT dismount;
        FILE_CALL_DEVICE_IO_CONTROL device_io_control;
        FILE_CALL_QUERY_DIRECTORY query_directory;
    } u;
} FILE_CALL;

#if defined(__cplusplus)
extern "C"
{
#endif
  OFC_CORE_LIB OFC_MESSAGE *of_file_call_create(OFC_VOID);
#if defined(__cplusplus)
}
#endif

#endif
