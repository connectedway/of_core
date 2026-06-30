/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_FILE_H__)
#define __OFC_FILE_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/config.h"
#include "ofc/handle.h"
#include "ofc/fstype.h"

/**
 * \{
 * \defgroup file Open Files File APIs
 * The Open Files File Facility provides Generic File Handling APIs to 
 * applications.
 *
 * Open File also provides a dispatch to platform specific File System and
 * File System specific APIs.  Lastly, the Open File Module provides 
 * abstractions for device mapping and URL specifications.  
 *
 * Open File APIs are modeled after the Windows file APIs.  The advantages
 * of the Windows file API are:
 * 
 * - handle abstraction
 * - ability to wait on multiple handles from multiple facilities
 * - Overlapped I/O modeling
 * - Ease of integration with SMB
 * - Robust API
 *
 * Since the Open File file APIs are modeled after the Windows APIs, the
 * definitive documentation for them is derived from the Windows API 
 * documentation.  The naming convention used by Open Files is that all
 * typedefs that are capitalized in the Windows API (eg. WIN32_FIND_DATAW)
 * are provided in open files with a "OFC_" prefix, as in OFC_WIN32_FIND_DATAW.
 * All function calls in the Windows API (eg. CreateFile) are provided in
 * Open Files with a "Ofc" prefix, as in OfcCreateFile.  Unless 
 * specifically noted, the functionality of the open files routines are
 * equivalent to the functionality in the Win32 APIs.  For the most part,
 * a Windows file application can be ported to an open files application by
 * applying these two translation rules.
 *
 * The definitive Windows documentation for the file apis can be found at
 * [https://docs.microsoft.com/en-us/windows/win32/api/fileapi/].
 *
 * The Open Files Files Related APIs consist of the following functions:
 *
 * Function | Description
 * ---------|------------
 * \ref OfcCreateFileW | Open or Create a File and Return Handle (Wide)
 * \ref OfcCreateFileA | Open or Create a File and Return Handle (Normal)
 * \ref OfcCreateFile  | Open or Create a File and Return Handle (Default)
 * \ref OfcCloseHandle | Close a File Handle
 * \ref OfcCreateDirectoryW | Create a Directory (Wide)
 * \ref OfcCreateDirectoryA | Create a Directory (Normal)
 * \ref OfcCreateDirectory  | Create a Directory (Deault)
 * \ref OfcDeleteFileW | Delete a File (Wide)
 * \ref OfcDeleteFileA | Delete a File (Normal)
 * \ref OfcDeleteFile  | Delete a File (Default)
 * \ref OfcRemoveDirectoryW | Delete a Directory (Wide)
 * \ref OfcRemoveDirectoryA | Delete a Directory (Normal)
 * \ref OfcRemoveDirectory  | Delete a Directory (Default)
 * \ref OfcFindFirstFileW   | Open a Search for File (Wide)
 * \ref OfcFindFirstFileA   | Open a Search for File (Normal)
 * \ref OfcFindFirstFile    | Open a Search for File (Default)
 * \ref OfcFindNextFileW    | Return Next File (Wide)
 * \ref OfcFindNextFileA    | Return Next File (Normal)
 * \ref OfcFindNextFile     | Return Next File (Default)
 * \ref OfcFindClose        | Closes a file search
 * \ref OfcFlushFileBuffers | Flush a File
 * \ref OfcGetFileAttributesExW | Get File Attributes by Name (Wide)
 * \ref OfcGetFileAttributesExA | Get File Attributes by Name (Normal)
 * \ref OfcGetFileAttributesEx  | Get File Attributes by Name (Default)
 * \ref OfcGetFileInformationByHandleEx | Get File Attributes by Handle
 * \ref OfcMoveFileW | Move File (Wide)
 * \ref OfcMoveFileA | Move File (Normal)
 * \ref OfcMoveFile  | Move File (Default)
 * \ref OfcReadFile | Read from a File
 * \ref OfcCreateOverlapped | Create an overlapped buffer handle
 * \ref OfcDestroyOverlapped | Destroy an overlapped buffer handle
 * \ref OfcSetOverlappedOffset | Set file offset in buffer handle
 * \ref OfcGetOverlappedResult | Get the result of a overlapped buffer
 * \ref OfcSetEndOfFile | Set End of File
 * \ref OfcSetFileAttributesW | Set File Attributes (Wide)
 * \ref OfcSetFileAttributesA | Set File Attributes (Normal)
 * \ref OfcSetFileAttributes  | Set File Attributes (Default)
 * \ref OfcSetFileInformationByHandle | Set File Attributes by Handle
 * \ref OfcSetFilePointer | Set File Pointer
 * \ref OfcWriteFile | Write File Buffer
 * \ref OfcTransactNamedPipe | Perform Named Pipe Transaction
 * \ref OfcGetLastFileError | Get Last File Error on Handle
 * \ref OfcGetLastError | Get Last Error on Process
 * \ref OfcGetLastFileNameErrorW | Get Last File Error on Name (Wide)
 * \ref OfcGetLastFileNameErrorA | Get Last File Error on Name (Normal)
 * \ref OfcGetLastFileNameError  | Get Last File Error on Name (Default)
 * \ref ofc_get_error_string | Translate error code to string
 * \ref OfcGetDiskFreeSpaceW | Get Disk Free Space (Wide)
 * \ref OfcGetDiskFreeSpaceA | Get Disk Free Space (Normal)
 * \ref OfcGetDiskFreeSpace  | Get Disk Free Space (Default)
 * \ref OfcGetVolumeInformationW | Get Volume Information (Wide)
 * \ref OfcGetVolumeInformationA | Get Volume Information (Normal)
 * \ref OfcGetVolumeInformation  | Get Volume Information (Normal)
 * \ref OfcUnlockFileEx | Unlock a file
 * \ref OfcLockFileEx | Lock a file
 * \ref OfcFileGetOverlappedEvent | Get Event for Overlapped Handle
 * \ref OfcFileGetOverlappedWaitQ | Get WaitQ of Overlapped Handle
 * \ref OfcDismountW | Dismount a share (Wide)
 * \ref OfcDismountA | Dismount a share (Normal)
 * \ref OfcDismount  | Dismount a share (Default)
 * \ref OfcDeviceIoControl | Issue ioctl
 */

/**
 * The maximum size of a path
 */
#define OFC_MAX_PATH 260

/**
 * \struct _OFC_FILETIME
 *
 * The 64 bit representation of a file time
 */
typedef struct _OFC_FILETIME {
    OFC_DWORD dwLowDateTime;    /**< The Low 32 bits  */
    OFC_DWORD dwHighDateTime;    /**< The High 32 bits  */
} OFC_FILETIME;

/**
 * Valid Access Rights for Files and Directories
 */
enum {
    /**
     * The right to create a file in the directory 
     */
    OFC_FILE_ADD_FILE = 0x0002, 
    /**
     * The right to create a subdirectory
     */
    OFC_FILE_ADD_SUBDIRECTORY = 0x0004,
    /**
     * All possible access rights for a file
     */
    OFC_FILE_ALL_ACCESS = 0xFFFF,
    /**
     * The right to append data to a file
     */
    OFC_FILE_APPEND_DATA = 0x0004,
    /**
     * The right to delete a directory
     */
    OFC_FILE_DELETE_CHILD = 0x0040,
    /**
     * The right to execute the file
     */
    OFC_FILE_EXECUTE = 0x0020,
    /**
     * The right to list contents of a directory
     */
    OFC_FILE_LIST_DIRECTORY = 0x0001,
    /**
     * The right to read file attributes
     */
    OFC_FILE_READ_ATTRIBUTES = 0x0080,
    /**
     * The right to read file data
     */
    OFC_FILE_READ_DATA = 0x0001,
    /**
     * The right to read extended file attributes
     */
    OFC_FILE_READ_EA = 0x0008,
    /**
     * The right to traverse a directory
     */
    OFC_FILE_TRAVERSE = 0x0020,
    /**
     * The right to write file attributes
     */
    OFC_FILE_WRITE_ATTRIBUTES = 0x0100,
    /**
     * The right to write data to a file
     */
    OFC_FILE_WRITE_DATA = 0x0002,
    /**
     * The right to write extended file attributes
     */
    OFC_FILE_WRITE_EA = 0x0010,
    /**
     * Delete Access
     */
    OFC_FILE_DELETE = 0x00010000L,
    /**
     * Generic Execute Access
     */
    OFC_GENERIC_EXECUTE =
    (OFC_FILE_READ_ATTRIBUTES |
     OFC_FILE_EXECUTE),
    /**
     * Generic Read Access
     */
    OFC_GENERIC_READ =
    (OFC_FILE_READ_ATTRIBUTES |
     OFC_FILE_READ_DATA |
     OFC_FILE_READ_EA),
    /**
     * Generic Write Access
     */
    OFC_GENERIC_WRITE =
    (OFC_FILE_APPEND_DATA |
     OFC_FILE_WRITE_ATTRIBUTES |
     OFC_FILE_WRITE_DATA |
     OFC_FILE_WRITE_EA),
};

/**
 * The Sharing Mode of an object
 */
enum {
    /**
     * Open file for exclusive access
     */
    OFC_FILE_SHARE_NONE = 0,
    /**
     * Allow subsequent opens to request delete access and allow this access if
     * file previously opened with delete access
     */
    OFC_FILE_SHARE_DELETE = 0x04,
    /**
     * Allow subsequent opens to request write access and allow this access if
     * file previously opened with write access
     */
    OFC_FILE_SHARE_WRITE = 0x02,
    /**
     * Allow subsequent opens to request read access and allow this access if 
     * file previously opened with read access
     */
    OFC_FILE_SHARE_READ = 0x01
};

/**
 * A action to take on files whether they exist or not.
 */
enum {
    /**
     * Create a New File.  Fail if file already exists
     */
    OFC_CREATE_NEW = 1,
    /**
     * Create a file always.  If it exists overwrite.
     */
    OFC_CREATE_ALWAYS = 2,
    /**
     * Open an Existing File.  If the file doesn't exist, fail
     */
    OFC_OPEN_EXISTING = 3,
    /**
     * Open a file always.  If the file doesn't exist, create
     */
    OFC_OPEN_ALWAYS = 4,
    /**
     * Open a file and truncate.  If the file doesn't exist, fail.
     */
    OFC_TRUNCATE_EXISTING = 5
};


/**
 * File Attributes and Flags
 */
enum {
    /**
     * The file is read only
     */
    OFC_FILE_ATTRIBUTE_READONLY = 0x0001,
    /**
     * The file is hidden
     */
    OFC_FILE_ATTRIBUTE_HIDDEN = 0x0002,
    /**
     * The file is a system file
     */
    OFC_FILE_ATTRIBUTE_SYSTEM = 0x0004,
    OFC_FILE_ATTRIBUTE_BOOKMARK = 0x0008,
    /**
     * The file is a directory
     */
    OFC_FILE_ATTRIBUTE_DIRECTORY = 0x0010,
    /**
     * The file should be archived
     */
    OFC_FILE_ATTRIBUTE_ARCHIVE = 0x0020,
    /**
     * The file is a normal file
     */
    OFC_FILE_ATTRIBUTE_NORMAL = 0x0080,
    /**
     * The file is a temporary file
     */
    OFC_FILE_ATTRIBUTE_TEMPORARY = 0x0100,
    /**
     * The file is a sparse file
     */
    OFC_FILE_ATTRIBUTE_SPARSE_FILE = 0x0200,

    OFC_FILE_ATTRIBUTE_REPARSE_POINT = 0x0400,

    /**
     * The file is compressed
     */
    OFC_FILE_ATTRIBUTE_COMPRESSED = 0x0800,
    /**
     * The file data is not immediate available
     */
    OFC_FILE_ATTRIBUTE_OFFLINE = 0x1000,

    OFC_FILE_ATTRIBUTE_NOT_CONTENT_INDEXED = 0x00002000,

    /**
     * The file is encrypted
     */
    OFC_FILE_ATTRIBUTE_ENCRYPTED = 0x4000,

    OFC_FILE_ATTRIBUTE_INTEGRITY_STREAM = 0x8000,

    /**
     * The file is virtual
     */
    OFC_FILE_ATTRIBUTE_VIRTUAL = 0x00010000,
    OFC_FILE_ATTRIBUTE_NO_SCRUB_DATA = 0x00020000,
    OFC_FILE_ATTRIBUTE_REFERRAL = 0x00040000,
    /**
     * If the file is remote, don't move back to local storage
     */
    OFC_FILE_FLAG_OPEN_NO_RECALL = 0x00100000,
    /*
     * These are Open Files Specific to denote remote files
     */
    /**
     * File is a workgroup directory
     */
    OFC_FILE_FLAG_WORKGROUP = 0x00200000,
    /**
     * File is a server directory
     */
    OFC_FILE_FLAG_SERVER = 0x00400000,
    /*
     * File is a Share directory
     */
    OFC_FILE_FLAG_SHARE = 0x00800000,
    /**
     * The file is being opened with Posix Semantics (i.e. case sensitive)
     */
    OFC_FILE_FLAG_POSIX_SEMANTICS = 0x01000000,
    /**
     * The file is opened for a backup or restore operation
     */
    OFC_FILE_FLAG_BACKUP_SEMANTICS = 0x02000000,
    /**
     * The file is to be deleted when closed
     */
    OFC_FILE_FLAG_DELETE_ON_CLOSE = 0x04000000,
    /**
     * The file will be accessed sequentially from beggining to end
     */
    OFC_FILE_FLAG_SEQUENTIAL_SCAN = 0x08000000,
    /**
     * The file is to be accessed randomly
     */
    OFC_FILE_FLAG_RANDOM_ACCESS = 0x10000000,
    /**
     * The file data should not be cached
     */
    OFC_FILE_FLAG_NO_BUFFERING = 0x20000000,
    /**
     * The file is being opened for asynchronous I/O
     */
    OFC_FILE_FLAG_OVERLAPPED = 0x40000000,
    /**
     * Write Caching should not occur
     */
    OFC_FILE_FLAG_WRITE_THROUGH = 0x80000000
};

/*
 * Create Options
 */
enum {
    /**
     * Create a directory
     */
    OFC_FILE_CREATE_DIRECTORY = 0x0001,
    /**
     * Do not cache writes
     */
    OFC_FILE_CREATE_WRITETHROUGH = 0x0002,
    /**
     * I/O is performed sequentially
     */
    OFC_FILE_CREATE_SEQUENTIAL = 0x0004,
    /**
     * Enable Alerts on the file
     */
    OFC_FILE_CREATE_SIOALERT = 0x0010,
    /**
     * Do not enable alerts on the file
     */
    OFC_FILE_CREATE_SIONONALERT = 0x0020,
    /**
     * This is not a directory
     */
    OFC_FILE_CREATE_NONDIRECTORY = 0x0040,
    /**
     * Do not create extended attributes
     */
    OFC_FILE_CREATE_NOEA = 0x0200,
    /**
     * Enforce 8x3 file naming
     */
    OFC_FILE_CREATE_8x3 = 0x0400,
    /**
     * Create a random file
     */
    OFC_FILE_CREATE_RANDOM = 0x0800,
    /**
     * Delete this file when closed
     */
    OFC_FILE_CREATE_DELONCLOSE = 0x1000
};

/*
 * Open Actions
 */
enum {
    /**
     * The file was opened
     */
    OFC_FILE_OPEN_ACTION_OPENED,
    /**
     * The file was created
     */
    OFC_FILE_OPEN_ACTION_CREATED,
    /**
     * The file was truncated
     */
    OFC_FILE_OPEN_ACTION_TRUNCATED
};

/**
 * Identifies the type of file information for the 
 * GetFileInformationByHandleEx or SetFileInformationByHandleEx call
 */
typedef enum _OFC_FILE_INFO_BY_HANDLE_CLASS {
    /**
     * Minimal Information
     */
    OfcFileBasicInfo = 0,
    /**
     * Extended Information
     */
    OfcFileStandardInfo,
    /**
     * All Information
     */
    OfcFileAllInfo,
    /**
     * The File Name
     */
    OfcFileNameInfo,
    /**
     * The File Name should be changed
     */
    OfcFileRenameInfo,
    /**
     * The File Should be deleted
     */
    OfcFileDispositionInfo,
    /**
     * File Allocation Info should be changed
     */
    OfcFileAllocationInfo,
    /**
     * The end of the file should be set
     */
    OfcFileEndOfFileInfo,
    /**
     * File Stream Information should be retrieved
     */
    OfcFileStreamInfo,
    /**
     * File Extended Attribute Information should be retrieved
     */
    OfcFileEaInfo,
    /**
     * File Compression Info should be retrieved
     */
    OfcFileCompressionInfo,
    /**
     * File Attribute Information Should be Retrieved
     */
    OfcFileAttributeTagInfo,
    /**
     * Files in the directory should be retrieved
     */
    OfcFileIdBothDirectoryInfo,
    /**
     * Identical fo FileIdBothDirectoryInfo but the enumeration should begin
     * from the beginning
     */
    OfcFileIdBothDirectoryRestartInfo,
    /*
     * We'll include this one too.  It's the one you can get without a file
     * handle 
     */
    OfcFileInfoStandard,
    /*
     * All
     */
    OfcFileInfoAll,
    /*
     */
    OfcFileNetworkOpenInfo,
    /*
     * Query Directory Values
     */
    OfcFileDirectoryInformation,
    OfcFileFullDirectoryInformation,
    OfcFileIdFullDirectoryInformation,
    OfcFileBothDirectoryInformation,
    OfcFileIdBothDirectoryInformation,
    OfcFileNamesInformation,
    OfcFileInternalInformation,
    /**
     * Maximum Value
     */
    OfcMaximumFileInfoByHandlesClass
} OFC_FILE_INFO_BY_HANDLE_CLASS;

typedef enum _OFC_FILEFS_INFO_BY_HANDLE_CLASS {
    OfcFileFSAttributeInformation,
    OfcFileFSSizeInformation,
    OfcFileFSVolumeInformation,
    OfcFileFSFullSizeInformation,
    OfcFileFSDeviceInformation
} OFC_FILEFS_INFO_BY_HANDLE_CLASS;

typedef enum _OFC_FILE_INFO_BY_HANDLE_TYPE {
    OfcFileInfoTypeFile,
    OfcFileInfoTypeFS,
    OfcFileInfoTypeSecurity,
    OfcFileInfoTypeQuota
} OFC_FILE_INFO_BY_HANDLE_TYPE;

/**
 * Security Attributes for files
 *
 * This is not used but is here for completeness
 */
typedef struct _OFC_SECURITY_ATTRIBUTES {
    /**
     * The length of the security descriptor
     */
    OFC_DWORD nLength;
    /**
     * A pointer to the security descriptor
     */
    OFC_LPVOID lpSecurityDescriptor;
    /**
     * Whether this security descriptor can be inherited
     */
    OFC_BOOL bInheritHandle;
} OFC_SECURITY_ATTRIBUTES;

/**
 * A pointer to a security descriptor
 */
typedef OFC_VOID *OFC_PSECURITY_DESCRIPTOR;
/**
 * A pointer to the security attributes
 */
typedef OFC_SECURITY_ATTRIBUTES *OFC_LPSECURITY_ATTRIBUTES;

/**
 * Contains information about the file that is found by OfcFindFirstFile,
 * OfcFindFirstFileEx or OfcFindNextFile
 */
typedef struct _OFC_WIN32_FIND_DATAW {
    /**
     * The attributes of the file
     */
    OFC_DWORD dwFileAttributes;
    /**
     * The create time of the file
     */
    OFC_FILETIME ftCreateTime;
    /**
     * The time the file was last accessed
     */
    OFC_FILETIME ftLastAccessTime;
    /**
     * The time the file was last written
     */
    OFC_FILETIME ftLastWriteTime;
    /**
     * The upper Dword value of the file size
     */
    OFC_DWORD nFileSizeHigh;
    /**
     * The low order value of the file size
     */
    OFC_DWORD nFileSizeLow;
    /**
     * Unused
     */
    OFC_DWORD dwReserved0;
    /**
     * Unused
     */
    OFC_DWORD dwReserved1;
    /**
     * The name of the file
     */
    OFC_TCHAR cFileName[OFC_MAX_PATH];
    /**
     * The 8.3 name of the file
     */
    OFC_TCHAR cAlternateFileName[14];

    OFC_LARGE_INTEGER FileId;

} OFC_WIN32_FIND_DATAW;
/**
 * A pointer to the Win32 Find data
 */
typedef OFC_WIN32_FIND_DATAW *OFC_LPWIN32_FIND_DATAW;

typedef struct _OFC_WIN32_FIND_DATAA {
    /**
     * The attributes of the file
     */
    OFC_DWORD dwFileAttributes;
    /**
     * The create time of the file
     */
    OFC_FILETIME ftCreateTime;
    /**
     * The time the file was last accessed
     */
    OFC_FILETIME ftLastAccessTime;
    /**
     * The time the file was last written
     */
    OFC_FILETIME ftLastWriteTime;
    /**
     * The upper Dword value of the file size
     */
    OFC_DWORD nFileSizeHigh;
    /**
     * The low order value of the file size
     */
    OFC_DWORD nFileSizeLow;
    /**
     * Unused
     */
    OFC_DWORD dwReserved0;
    /**
     * Unused
     */
    OFC_DWORD dwReserved1;
    /**
     * The name of the file
     */
    OFC_CHAR cFileName[OFC_MAX_PATH];
    /**
     * The 8.3 name of the file
     */
    OFC_CHAR cAlternateFileName[14];
} OFC_WIN32_FIND_DATAA;
/**
 * A pointer to the Win32 Find data
 */
typedef OFC_WIN32_FIND_DATAA *OFC_LPWIN32_FIND_DATAA;

/**
 * Contains information about the file that is found by OfcFindFirstFile,
 * OfcFindFirstFileEx or OfcFindNextFile
 */
typedef struct _OFC_WIN32_FILE_ATTRIBUTE_DATA {
    /**
     * The attributes of the file
     */
    OFC_DWORD dwFileAttributes;
    /**
     * The create time of the file
     */
    OFC_FILETIME ftCreateTime;
    /**
     * The time the file was last accessed
     */
    OFC_FILETIME ftLastAccessTime;
    /**
     * The time the file was last written
     */
    OFC_FILETIME ftLastWriteTime;
    /**
     * The upper Dword value of the file size
     */
    OFC_DWORD nFileSizeHigh;
    /**
     * The low order value of the file size
     */
    OFC_DWORD nFileSizeLow;
} OFC_WIN32_FILE_ATTRIBUTE_DATA;

/**
 * A pointer to the Win32 File Attribute Data
 */
typedef OFC_WIN32_FILE_ATTRIBUTE_DATA *OFC_LPWIN32_FILE_ATTRIBUTE_DATA;

/**
 * Values for the OfcGetFileAttributesEx Call
 */
typedef enum _OFC_GET_FILEEX_INFO_LEVELS {
    /**
     * Standard Information
     */
    OfcGetFileExInfoStandard,
    /**
     * Max Value
     */
    OfcGetFileExMaxInfoLevel
} OFC_GET_FILEEX_INFO_LEVELS;

/**
 * Contains information used in asynchrous I/O
 */
typedef struct _OFC_OVERLAPPED {
    OFC_BOOL status;
    OFC_HANDLE hFile;
    OFC_DWORD dwLen;
    OFC_OFFT offset;
    OFC_HANDLE hContext;
    OFC_HANDLE response_queue;
#if defined(OFC_PERF_STATS)
    OFC_INT perf_id;
    OFC_MSTIME stamp;
#endif
} OFC_OVERLAPPED;

typedef OFC_OVERLAPPED *OFC_LPOVERLAPPED;

/**
 * Basic Information for a file
 */
typedef struct _OFC_FILE_BASIC_INFO {
    /**
     * The time fthe file was created
     */
    OFC_LARGE_INTEGER CreationTime;
    /**
     * The time the file was last accessed
     */
    OFC_LARGE_INTEGER LastAccessTime;
    /**
     * The time the file was last written to
     */
    OFC_LARGE_INTEGER LastWriteTime;
    /**
     * The time the file was changed
     */
    OFC_LARGE_INTEGER ChangeTime;
    /**
     * The File Attributes
     */
    OFC_DWORD FileAttributes;
} OFC_FILE_BASIC_INFO;

#define OFC_FILE_BASIC_CREATION_TIME 0
#define OFC_FILE_BASIC_LAST_ACCESS_TIME 8
#define OFC_FILE_BASIC_LAST_WRITE_TIME 16
#define OFC_FILE_BASIC_CHANGE_TIME 24
#define OFC_FILE_BASIC_ATTRIBUTES 32
#define OFC_FILE_BASIC_SIZE 36

typedef struct _OFC_FILE_INTERNAL_INFO {
    OFC_LARGE_INTEGER IndexNumber;
} OFC_FILE_INTERNAL_INFO;

typedef struct _OFC_FILE_EA_INFO {
    OFC_DWORD EaSize;
} OFC_FILE_EA_INFO;

typedef struct _OFC_FILE_ACCESS_INFO {
    OFC_DWORD AccessFlags;
} OFC_FILE_ACCESS_INFO;

typedef struct _OFC_FILE_POSITION_INFO {
    OFC_LARGE_INTEGER CurrentByteOffset;
} OFC_FILE_POSITION_INFO;

typedef struct _OFC_FILE_MODE_INFO {
    OFC_DWORD Mode;
} OFC_FILE_MODE_INFO;

typedef struct _OFC_FILE_ALIGNMENT_INFO {
    OFC_DWORD AlignmentRequirement;
} OFC_FILE_ALIGNMENT_INFO;

/**
 * Network Open Information for a file
 */
typedef struct _OFC_FILE_NETWORK_OPEN_INFO {
    /**
     * The time fthe file was created
     */
    OFC_LARGE_INTEGER CreationTime;
    /**
     * The time the file was last accessed
     */
    OFC_LARGE_INTEGER LastAccessTime;
    /**
     * The time the file was last written to
     */
    OFC_LARGE_INTEGER LastWriteTime;
    /**
     * The time the file was changed
     */
    OFC_LARGE_INTEGER ChangeTime;
    /**
     * The Alloction Size
     */
    OFC_LARGE_INTEGER AllocationSize;
    /**
     * The End of File
     */
    OFC_LARGE_INTEGER EndOfFile;
    /**
     * The File Attributes
     */
    OFC_DWORD FileAttributes;
} OFC_FILE_NETWORK_OPEN_INFO;

/**
 * Extended Information for a file
 */
typedef struct _OFC_FILE_STANDARD_INFO {
    /**
     * The amount of space that is allocated for the file
     */
    OFC_LARGE_INTEGER AllocationSize;
    /**
     * The end of the file
     */
    OFC_LARGE_INTEGER EndOfFile;
    /**
     * The number of links to the file
     */
    OFC_DWORD NumberOfLinks;
    /**
     * OFC_TRUE if the file is being deleted
     */
    OFC_BOOL DeletePending;
    /**
     * OFC_TRUE if the file is a directory
     */
    OFC_BOOL Directory;
} OFC_FILE_STANDARD_INFO;

#define OFC_FILE_STANDARD_ALLOCATION_SIZE 0
#define OFC_FILE_STANDARD_END_OF_FILE 8
#define OFC_FILE_STANDARD_NUMBER_OF_LINKS 16
#define OFC_FILE_STANDARD_DELETE_PENDING 20
#define OFC_FILE_STANDARD_DIRECTORY 21
#define OFC_FILE_STANDARD_SIZE 22

/**
 * Receives the File Name
 */
typedef struct _OFC_FILE_NAME_INFO {
    /**
     * The size of the FileName string
     */
    OFC_DWORD FileNameLength;
    /**
     * The file name
     */
    OFC_WCHAR FileName[1];
} OFC_FILE_NAME_INFO;

/**
 * Contains the name to which the file should be renamed
 */
typedef struct _OFC_FILE_RENAME_INFO {
    /**
     * OFC_TRUE to replace an existing file if it exists
     */
    OFC_BOOL ReplaceIfExists;
    /**
     * A Handle to the root directory of the file to be renamed
     */
    OFC_HANDLE RootDirectory;
    /**
     * The size of the FileName
     */
    OFC_DWORD FileNameLength;
    /**
     * The New File Name
     */
    OFC_WCHAR FileName[1];
} OFC_FILE_RENAME_INFO;

/** 
 * Indicates whether a file should be deleted
 */
typedef struct _OFC_FILE_DISPOSITION_INFO {
    /**
     * Indicates whether the file should be deleted.  OFC_TRUE if it should
     */
    OFC_BOOL DeleteFile;
} OFC_FILE_DISPOSITION_INFO;

/**
 * Contains the total number of bytes that should be allocated for a file
 */
typedef struct _OFC_FILE_ALLOCATION_INFO {
    /**
     * The new file allocation size
     */
    OFC_LARGE_INTEGER AllocationSize;
} OFC_FILE_ALLOCATION_INFO;

/** 
 * Contains the specified value to which the end of file should be set
 */
typedef struct _OFC_FILE_END_OF_FILE_INFO {
    /**
     * The specified value for the new end of file
     */
    OFC_LARGE_INTEGER EndOfFile;
} OFC_FILE_END_OF_FILE_INFO;

/**
 * Receives file stream info for the file
 */
typedef struct _OFC_FILE_STREAM_INFO {
    /**
     * The offset for the next FILE_STREAM_INFO entry
     */
    OFC_DWORD NextEntryOffset;
    /**
     * The length in bytes of the Stream Name
     */
    OFC_DWORD StreamNameLength;
    /**
     * The size of the data stream
     */
    OFC_LARGE_INTEGER StreamSize;
    /**
     * The amount of space allocated for the stream
     */
    OFC_LARGE_INTEGER StreamAllocationSize;
    /**
     * The Stream Name
     */
    OFC_WCHAR StreamName[1];
} OFC_FILE_STREAM_INFO;

/**
 * Receives file compression information
 */
typedef struct _OFC_FILE_COMPRESSION_INFO {
    /**
     * The file size of the compressed file
     */
    OFC_LARGE_INTEGER CompressedFileSize;
    /**
     * The compression format
     */
    OFC_WORD CompressionFormat;
    /**
     * The factor that the compression uses
     */
    OFC_UCHAR CompressionUnitShift;
    /**
     * The number of chuncks that are shifted by compression
     */
    OFC_UCHAR ChunkShift;
    /**
     * The number of clusters that are shifted
     */
    OFC_UCHAR ClusterShift;
    /**
     * Reserved
     */
    OFC_UCHAR Reserved[3];
} OFC_FILE_COMPRESSION_INFO;

/**
 * Receives the requested file attribute information
 */
typedef struct _OFC_FILE_ATTRIBUTE_TAG_INFO {
    /**
     * The file attribute information
     */
    OFC_DWORD FileAttributes;
    /**
     * The reparse tag
     */
    OFC_DWORD ReparseTag;
} OFC_FILE_ATTRIBUTE_TAG_INFO;

/**
 * Contains Information about files in a specified directory
 */
typedef struct _OFC_FILE_DIR_INFO {
    /**
     * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
     */
    OFC_DWORD NextEntryOffset;
    /**
     * The byte offset of the file within the parent directory
     */
    OFC_DWORD FileIndex;
    /**
     * The time that the file was created
     */
    OFC_LARGE_INTEGER CreationTime;
    /**
     * The time that the file was last accessed
     */
    OFC_LARGE_INTEGER LastAccessTime;
    /**
     * The time that the file was last written to
     */
    OFC_LARGE_INTEGER LastWriteTime;
    /**
     * The time that the file was last changed
     */
    OFC_LARGE_INTEGER ChangeTime;
    /**
     * The absolute end of file position.
     */
    OFC_LARGE_INTEGER EndOfFile;
} OFC_FILE_DIR_INFO;

/**
 * Contains Information about files in a specified directory
 */
typedef struct _OFC_FILE_FULL_DIR_INFO {
    /**
     * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
     */
    OFC_DWORD NextEntryOffset;
    /**
     * The byte offset of the file within the parent directory
     */
    OFC_DWORD FileIndex;
    /**
     * The time that the file was created
     */
    OFC_LARGE_INTEGER CreationTime;
    /**
     * The time that the file was last accessed
     */
    OFC_LARGE_INTEGER LastAccessTime;
    /**
     * The time that the file was last written to
     */
    OFC_LARGE_INTEGER LastWriteTime;
    /**
     * The time that the file was last changed
     */
    OFC_LARGE_INTEGER ChangeTime;
    /**
     * The absolute end of file position.
     */
    OFC_LARGE_INTEGER EndOfFile;
    /**
     * The number of bytes allocated for the file
     */
    OFC_LARGE_INTEGER AllocationSize;
    /**
     * The file attributes
     */
    OFC_DWORD FileAttributes;
    /**
     * The length of the file name
     */
    OFC_DWORD FileNameLength;
    /**
     * The size of the extended attributes for the file
     */
    OFC_DWORD EaSize;
} OFC_FILE_FULL_DIR_INFO;

/**
 * Contains Information about files in a specified directory
 */
typedef struct _OFC_FILE_ID_FULL_DIR_INFO {
    /**
     * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
     */
    OFC_DWORD NextEntryOffset;
    /**
     * The byte offset of the file within the parent directory
     */
    OFC_DWORD FileIndex;
    /**
     * The time that the file was created
     */
    OFC_LARGE_INTEGER CreationTime;
    /**
     * The time that the file was last accessed
     */
    OFC_LARGE_INTEGER LastAccessTime;
    /**
     * The time that the file was last written to
     */
    OFC_LARGE_INTEGER LastWriteTime;
    /**
     * The time that the file was last changed
     */
    OFC_LARGE_INTEGER ChangeTime;
    /**
     * The absolute end of file position.
     */
    OFC_LARGE_INTEGER EndOfFile;
    /**
     * The number of bytes allocated for the file
     */
    OFC_LARGE_INTEGER AllocationSize;
    /**
     * The file attributes
     */
    OFC_DWORD FileAttributes;
    /**
     * The length of the file name
     */
    OFC_DWORD FileNameLength;
    /**
     * The size of the extended attributes for the file
     */
    OFC_DWORD EaSize;
    /**
     * Reserved
     */
    OFC_DWORD Reserved;
    /**
     * The File ID
     */
    OFC_LARGE_INTEGER FileId;
    /**
     * The File Name
     */
    OFC_WCHAR FileName[1];
} OFC_FILE_ID_FULL_DIR_INFO;

/**
 * Contains Information about files in a specified directory
 */
typedef struct _OFC_FILE_BOTH_DIR_INFO {
    /**
     * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
     */
    OFC_DWORD NextEntryOffset;
    /**
     * The byte offset of the file within the parent directory
     */
    OFC_DWORD FileIndex;
    /**
     * The time that the file was created
     */
    OFC_LARGE_INTEGER CreationTime;
    /**
     * The time that the file was last accessed
     */
    OFC_LARGE_INTEGER LastAccessTime;
    /**
     * The time that the file was last written to
     */
    OFC_LARGE_INTEGER LastWriteTime;
    /**
     * The time that the file was last changed
     */
    OFC_LARGE_INTEGER ChangeTime;
    /**
     * The absolute end of file position.
     */
    OFC_LARGE_INTEGER EndOfFile;
    /**
     * The number of bytes allocated for the file
     */
    OFC_LARGE_INTEGER AllocationSize;
    /**
     * The file attributes
     */
    OFC_DWORD FileAttributes;
    /**
     * The length of the file name
     */
    OFC_DWORD FileNameLength;
    /**
     * The size of the extended attributes for the file
     */
    OFC_DWORD EaSize;
    /**
     * The length of the short name (8.3 name)
     */
    OFC_CHAR ShortNameLength;
    /**
     * Reserved
     */
    OFC_CHAR Reserved;
    /**
     * The short 8.3 file name
     */
    OFC_WCHAR ShortName[12];
    /**
     * The File Name
     */
    OFC_WCHAR FileName[1];
} OFC_FILE_BOTH_DIR_INFO;

/**
 * Contains Information about files in a specified directory
 */
typedef struct _OFC_FILE_ID_BOTH_DIR_INFO {
    /**
     * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
     */
    OFC_DWORD NextEntryOffset;
    /**
     * The byte offset of the file within the parent directory
     */
    OFC_DWORD FileIndex;
    /**
     * The time that the file was created
     */
    OFC_LARGE_INTEGER CreationTime;
    /**
     * The time that the file was last accessed
     */
    OFC_LARGE_INTEGER LastAccessTime;
    /**
     * The time that the file was last written to
     */
    OFC_LARGE_INTEGER LastWriteTime;
    /**
     * The time that the file was last changed
     */
    OFC_LARGE_INTEGER ChangeTime;
    /**
     * The absolute end of file position.
     */
    OFC_LARGE_INTEGER EndOfFile;
    /**
     * The number of bytes allocated for the file
     */
    OFC_LARGE_INTEGER AllocationSize;
    /**
     * The file attributes
     */
    OFC_DWORD FileAttributes;
    /**
     * The length of the file name
     */
    OFC_DWORD FileNameLength;
    /**
     * The size of the extended attributes for the file
     */
    OFC_DWORD EaSize;
    /**
     * The length of the short name (8.3 name)
     */
    OFC_CHAR ShortNameLength;
    /**
     * The short 8.3 file name
     */
    OFC_WCHAR ShortName[12];
    /**
     * The File ID
     */
    OFC_LARGE_INTEGER FileId;
    /**
     * The File Name
     */
    OFC_WCHAR FileName[1];
} OFC_FILE_ID_BOTH_DIR_INFO;

typedef struct _OFC_FILE_ALL_INFO {
    OFC_FILE_BASIC_INFO BasicInfo;
    OFC_FILE_STANDARD_INFO StandardInfo;
    OFC_FILE_INTERNAL_INFO InternalInfo;
    OFC_FILE_EA_INFO EAInfo;
    OFC_FILE_ACCESS_INFO AccessInfo;
    OFC_FILE_POSITION_INFO PositionInfo;
    OFC_FILE_MODE_INFO ModeInfo;
    OFC_FILE_ALIGNMENT_INFO AlignmentInfo;
    OFC_FILE_NAME_INFO NameInfo;
} OFC_FILE_ALL_INFO;

/*
 * Access Mask Encodings
 */
#define OFC_DELETE 0x00010000
#define OFC_READ_CONTROL 0x00020000
#define OFC_WRITE_DAC 0x00040000
#define OFC_WRITE_OWNER 0x00080000
#define OFC_SYNCHRONIZE 0x00100000
#define OFC_ACCESS_SYSTEM_SECURITY 0x01000000
#define OFC_MAXIMUM_ALLOWED 0x02000000
#define OFC_GENERIC_ALL 0x10000000
#if 0
#define OFC_GENERIC_EXECUTE 0x20000000
#define OFC_GENERIC_WRITE 0x40000000
#define OFC_GENERIC_READ 0x80000000
#endif

/**
 * Contains Information about files in a specified directory
 */
typedef struct _OFC_FILE_NAMES_INFO {
    /**
     * The offset for the next FILE_ID_BOTH_DIR_INFO structure in the buffer
     */
    OFC_DWORD NextEntryOffset;
    /**
     * The byte offset of the file within the parent directory
     */
    OFC_DWORD FileIndex;
    /**
     * The length of the file name
     */
    OFC_DWORD FileNameLength;
    /**
     * The File Name
     */
    OFC_WCHAR FileName[1];
} OFC_FILE_NAMES_INFO;

/**
 * Defines values for File I/O Priority
 */
typedef enum _OFC_PRIORITY_HINT {
    OfcIoPriorityHintVeryLow = 0,
    OfcIoPriorityHintLow,
    OfcIoPriorityHintNormal,
    OfcMaximumIoPriorityHintType
} OFC_PRIORITY_HINT;

/**
 * Specifies the priority hint for a file I/O operation
 */
typedef struct _OFC_FILE_IO_PRIORITY_HINT_INFO {
    /**
     * The pirority hint
     */
    OFC_PRIORITY_HINT PriorityHint;
} OFC_FILE_IO_PRIORITY_HINT_INFO;

/* The MS-FSCC spec defines the following.  For some reason, I get
 * something different back
 */
typedef struct _OFC_FILEFS_SIZE_INFO {
    OFC_LARGE_INTEGER TotalAllocationUnits;
    OFC_LARGE_INTEGER AvailableAllocationUnits;
    OFC_UINT32 SectorsPerAllocationUnit;
    OFC_UINT32 BytesPerSector;
} OFC_FILEFS_SIZE_INFO;

#define FILEFS_SIZE_TOTAL_ALLOCATION_UNITS 0
#define FILEFS_SIZE_AVAILABLE_ALLOCATION_UNITS 8
#define FILEFS_SIZE_SECTORS_PER_ALLOCATION_UNIT 16
#define FILEFS_SIZE_BYTES_PER_SECTOR 20
#define FILEFS_SIZE_SIZE 24

typedef struct _OFC_FILEFS_FULL_SIZE_INFO {
    OFC_LARGE_INTEGER TotalAllocationUnits;
    OFC_LARGE_INTEGER CallerAvailableAllocationUnits;
    OFC_LARGE_INTEGER ActualAvailableAllocationUnits;
    OFC_UINT32 SectorsPerAllocationUnit;
    OFC_UINT32 BytesPerSector;
} OFC_FILEFS_FULL_SIZE_INFO;

typedef struct _OFC_FILEFS_ATTRIBUTE_INFO {
    OFC_UINT32 FileSystemAttributes;
    OFC_UINT32 MaximumComponentNameLength;
    OFC_UINT32 FileSystemNameLength;
} OFC_FILEFS_ATTRIBUTE_INFO;

#define OFC_FILEFS_FILE_SYSTEM_ATTRIBUTES 0
#define OFC_FILEFS_MAX_COMPONENT_NAME_LENGTH 4
#define OFC_FILEFS_FILE_SYSTEM_NAME_LENGTH 8
#define OFC_FILEFS_FILE_SYSTEM_NAME 12

/*
 * Defines for FilesystemInfo FSAttributes
 */
#define OFC_FILEFS_SUPPORTS_OBJECT_IDS 0x00010000
#define OFC_FILEFS_SUPPORTS_SPARSE_FILES 0x00000040
#define OFC_FILEFS_VOLUME_QUOTAS 0x00000020
#define OFC_FILEFS_PERSISTENT_ACLS 0x00000008
#define OFC_FILEFS_UNICODE_ON_DISK 0x00000004
#define OFC_FILEFS_CASE_PRESERVED_NAMES 0x00000002
#define OFC_FILEFS_CASE_SENSITIVE_SEARCH 0x00000001

typedef struct _OFC_FILEFS_VOLUME_INFO {
    OFC_LARGE_INTEGER VolumeCreationTime;
    OFC_UINT32 VolumeSerialNumber;
    OFC_UINT32 VolumeLabelLength;
    OFC_UINT8 SupportsObjects;
    OFC_UINT8 Reserved;
} OFC_FILEFS_VOLUME_INFO;

#define OFC_FILEFS_VOLUME_CREATION_TIME 0
#define OFC_FILEFS_VOLUME_SERIAL_NUMBER 8
#define OFC_FILEFS_VOLUME_LABEL_LENGTH 12
#define OFC_FILEFS_SUPPORTS_OBJECTS 16
#define OFC_FILEFS_RESERVED 17
#define OFC_FILEFS_VOLUME_LABEL 18

typedef struct _OFC_FILEFS_DEVICE_INFO {
    OFC_UINT32 DeviceType;
    OFC_UINT32 Characteristics;
} OFC_FILEFS_DEVICE_INFO;

#define OFC_FILE_DEVICE_CD_ROM 2
#define OFC_FILE_DEVICE_DISK 7

#define OFC_FILE_REMOVABLE_MEDIA 0x01
#define OFC_FILE_READ_ONLY_DEVICE 0x02
#define OFC_FILE_FLOPPY_DISKETTE 0x04
#define OFC_FILE_WRITE_ONCE_MEDIA 0x08
#define OFC_FILE_REMOTE_DEVICE 0x10
#define OFC_FILE_DEVICE_IS_MOUNTED 0x20
#define OFC_FILE_VIRTUAL_VOLUME 0x40
#define OFC_FILE_DEVICE_SECURE_OPEN 0x100
#define OFC_FILE_CHARACTERISTIC_TS_DEVICE 0x1000
#define OFC_FILE_CHARACTERISTIC_WEBDEV_DEVICE 0x2000
#define OFC_FILE_DEVICE_ALLOW_APPCONTAINER_TRAVERSAL 0x20000
#define OFC_FILE_PORTABLE_DEVICE 0x4000

typedef struct _OFC_FILESECURITY_DESCRIPTOR {
  OFC_UINT8 Revision;
  OFC_UINT8 Sbz1;
  OFC_UINT16 Control;
  OFC_UINT32 OffsetOwner;
  OFC_UINT32 OffsetGroup;
  OFC_UINT32 OffsetSacl;
  OFC_UINT32 OffsetDacl;
} OFC_FILESECURITY_DESCRIPTOR;

#define SECURITY_DESCRIPTOR_CONTROL_SR 0x8000
#define SECURITY_DESCRIPTOR_CONTROL_RM 0x4000
#define SECURITY_DESCRIPTOR_CONTROL_PS 0x2000
#define SECURITY_DESCRIPTOR_CONTROL_PD 0x1000
#define SECURITY_DESCRIPTOR_CONTROL_SI 0x0800
#define SECURITY_DESCRIPTOR_CONTROL_DI 0x0400
#define SECURITY_DESCRIPTOR_CONTROL_SC 0x0200
#define SECURITY_DESCRIPTOR_CONTROL_DC 0x0100
#define SECURITY_DESCRIPTOR_CONTROL_DT 0x0080
#define SECURITY_DESCRIPTOR_CONTROL_SS 0x0040
#define SECURITY_DESCRIPTOR_CONTROL_SD 0x0020
#define SECURITY_DESCRIPTOR_CONTROL_SP 0x0010
#define SECURITY_DESCRIPTOR_CONTROL_DD 0x0008
#define SECURITY_DESCRIPTOR_CONTROL_DP 0x0004
#define SECURITY_DESCRIPTOR_CONTROL_GD 0x0002
#define SECURITY_DESCRIPTOR_CONTROL_OD 0x0001

/**
 * Enumerator for Move Method
 */
enum {
    /**
     * Move relative from the beginning of the file
     */
    OFC_FILE_BEGIN = 0,
    /**
     * Move relative to the current file pointer
     */
    OFC_FILE_CURRENT = 1,
    /**
     * Move relative to EOF
     */
    OFC_FILE_END = 2
};

/**
 * Flags for OfcLockFileEx
 */
enum {
    OFC_LOCKFILE_FAIL_IMMEDIATELY = 0x01,
    OFC_LOCKFILE_EXCLUSIVE_LOCK = 0x02
};

/**
 * Value returned to OfcSetFilePointer function when the pointer could not
 * be set
 */
#define OFC_INVALID_SET_FILE_POINTER ((OFC_DWORD)-1)

/**
 * Error Codes that can be returned by File APIs
 */
typedef enum {
    OFC_ERROR_SUCCESS = 0,
    OFC_ERROR_INVALID_FUNCTION = 1,
    OFC_ERROR_FILE_NOT_FOUND = 2,
    OFC_ERROR_PATH_NOT_FOUND = 3,
    OFC_ERROR_TOO_MANY_OPEN_FILES = 4,
    OFC_ERROR_ACCESS_DENIED = 5,
    OFC_ERROR_INVALID_HANDLE = 6,
    OFC_ERROR_NOT_ENOUGH_MEMORY = 8,
    OFC_ERROR_INVALID_ACCESS = 12,
    OFC_ERROR_OUTOFMEMORY = 14,
    OFC_ERROR_INVALID_DRIVE = 15,
    OFC_ERROR_CURRENT_DIRECTORY = 16,
    OFC_ERROR_NOT_SAME_DEVICE = 17,
    OFC_ERROR_NO_MORE_FILES = 18,
    OFC_ERROR_WRITE_PROTECT = 19,
    OFC_ERROR_NOT_READY = 21,
    OFC_ERROR_CRC = 23,
    OFC_ERROR_BAD_LENGTH = 24,
    OFC_ERROR_SEEK = 25,
    OFC_ERROR_WRITE_FAULT = 29,
    OFC_ERROR_READ_FAULT = 30,
    OFC_ERROR_GEN_FAILURE = 31,
    OFC_ERROR_SHARING_VIOLATION = 32,
    OFC_ERROR_LOCK_VIOLATION = 33,
    OFC_ERROR_WRONG_DISK = 34,
    OFC_ERROR_SHARING_BUFFER_EXCEEDED = 35,
    OFC_ERROR_HANDLE_EOF = 38,
    OFC_ERROR_HANDLE_DISK_FULL = 39,
    OFC_ERROR_BAD_NET_NAME = 43,
    OFC_ERROR_NOT_SUPPORTED = 50,
    OFC_ERROR_REM_NOT_LIST = 51,
    OFC_ERROR_DUP_NAME = 52,
    OFC_ERROR_BAD_NETPATH = 53,
    OFC_ERROR_NETWORK_BUSY = 54,
    OFC_ERROR_DEV_NOT_EXIST = 55,
    OFC_ERROR_BAD_NET_RESP = 58,
    OFC_ERROR_UNEXP_NET_ERR = 59,
    OFC_ERROR_BAD_DEV_TYPE = 66,
    OFC_ERROR_FILE_EXISTS = 80,
    OFC_ERROR_CANNOT_MAKE = 82,
    OFC_ERROR_INVALID_PASSWORD = 86,
    OFC_ERROR_INVALID_PARAMETER = 87,
    OFC_ERROR_NET_WRITE_FAULT = 88,
    OFC_ERROR_MORE_ENTRIES = 105,
    OFC_ERROR_BROKEN_PIPE = 109,
    OFC_ERROR_OPEN_FAILED = 110,
    OFC_ERROR_BUFFER_OVERFLOW = 111,
    OFC_ERROR_DISK_FULL = 112,
    OFC_ERROR_CALL_NOT_IMPLEMENTED = 120,
    OFC_ERROR_INSUFFICIENT_BUFFER = 122,
    OFC_ERROR_INVALID_NAME = 123,
    OFC_ERROR_INVALID_LEVEL = 124,
    OFC_ERROR_NO_VOLUME_LABEL = 125,
    OFC_ERROR_NEGATIVE_SEEK = 131,
    OFC_ERROR_SEEK_ON_DEVICE = 132,
    OFC_ERROR_DIR_NOT_EMPTY = 145,
    OFC_ERROR_PATH_BUSY = 148,
    OFC_ERROR_BAD_ARGUMENTS = 160,
    OFC_ERROR_BAD_PATHNAME = 161,
    OFC_ERROR_BUSY = 170,
    OFC_ERROR_ALREADY_EXISTS = 183,
    OFC_ERROR_INVALID_FLAG_NUMBER = 186,
    OFC_ERROR_BAD_PIPE = 230,
    OFC_ERROR_PIPE_BUSY = 231,
    OFC_ERROR_NO_DATA = 232,
    OFC_ERROR_PIPE_NOT_CONNECTED = 233,
    OFC_ERROR_MORE_DATA = 234,
    OFC_ERROR_INVALID_EA_NAME = 254,
    OFC_ERROR_EA_LIST_INCONSISTENT = 255,
    OFC_ERROR_DIRECTORY = 267,
    OFC_ERROR_EAS_DIDNT_FIT = 275,
    OFC_ERROR_EA_FILE_CORRUPT = 276,
    OFC_ERROR_EA_TABLE_FULL = 277,
    OFC_ERROR_INVALID_EA_HANDLE = 278,
    OFC_ERROR_EAS_NOT_SUPPORTED = 282,
    OFC_ERROR_OPLOCK_NOT_GRANTED = 300,
    OFC_ERROR_DISK_TOO_FRAGMENTED = 302,
    OFC_ERROR_DELETE_PENDING = 303,
    OFC_ERROR_PIPE_CONNECTED = 535,
    OFC_ERROR_PIPE_LISTENING = 536,
    OFC_ERROR_EA_ACCESS_DENIED = 994,
    OFC_ERROR_OPERATION_ABORTED = 995,
    OFC_ERROR_IO_INCOMPLETE = 996,
    OFC_ERROR_IO_PENDING = 997,
    OFC_ERROR_NOACCESS = 998,
    OFC_ERROR_INVALID_FLAGS = 1004,
    OFC_ERROR_UNRECOGNIZED_VOLUME = 1005,
    OFC_ERROR_FILE_INVALID = 1006,
    OFC_ERROR_NOTIFY_ENUM_DIR = 1022,
    OFC_ERROR_BUS_RESET = 1111,
    OFC_ERROR_IO_DEVICE = 1117,
    OFC_ERROR_DISK_OPERATION_FAILED = 1127,
    OFC_ERROR_BAD_DEVICE = 1200,
    OFC_ERROR_INVALID_PASSWORDNAME = 1215,
    OFC_ERROR_LOGON_FAILURE = 1326,
    OFC_ERROR_NOT_ENOUGH_QUOTA = 1816,
    OFC_ERROR_CONNECTION_REFUSED = 1225,
    OFC_ERROR_NETWORK_UNREACHABLE = 1231,
    OFC_ERROR_PROTOCOL_UNREACHABLE = 1233,
    OFC_ERROR_INVALID_LOGON_HOURS = 1328,
    OFC_ERROR_INVALID_WORKSTATION = 1329,
    OFC_ERROR_PASSWORD_EXPIRED = 1330,
    OFC_ERROR_ACCOUNT_DISABLED = 1331,
    OFC_ERROR_LOGON_NOT_GRANTED = 1380,
    OFC_ERROR_TIMEOUT = 1460,
    OFC_ERROR_ACCOUNT_EXPIRED = 1793,
    OFC_ERROR_ACCOUNT_LOCKED_OUT = 1909
} OFC_FILE_ERRORS;

/**
 * File System Global Last Error
 */
extern OFC_DWORD OfcLastError;

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Initialize the Open File Redirector
 *
 * This should only be called by the Framework Init Layer
 */
OFC_CORE_LIB OFC_VOID
OfcFileInit(OFC_VOID);
/**
 * Initialize File Local Storage for Threads
 */
OFC_CORE_LIB OFC_VOID
OfcFileThreadInit(OFC_VOID);
/**
 * Deinitialize File Local Storage for Threads
 */
OFC_CORE_LIB OFC_VOID
OfcFileThreadDeinit(OFC_VOID);

OFC_CORE_LIB OFC_VOID
OfcFileDestroy(OFC_VOID);
/**
 * Close A File Handle
 *
 * Use this call to release a file handle obtained from the Create call
 *
 * \param hObject
 * The handle to close
 *
 * \returns
 * OFC_TRUE if the function succeeded, OFC_FALSE if failure
 */
OFC_CORE_LIB OFC_BOOL
OfcCloseHandle(OFC_HANDLE hObject);
/**
 * Create or Open a File
 *
 * Use this call to obtain a file handle for a new or existing file
 *
 * \param lpFileName
 * The name of the object to be crated or opened.
 *
 * \param dwDesiredAccess
 * The access to the object, which can be read, write, or both.
 *
 * \param dwShareMode
 * The sharing mode of an object which can be read, write, both or none.
 *
 * \param lpSecurityAttributes
 * This parameter is ignored and should be NULL.
 *
 * \param dwCreationDisposition
 *  An action to take on files that exist or do not exist.
 *
 * \param dwFlagsAndAttributes
 * The file attributes and flag.  The flags used by Win32 are accepted.
 *
 * \param hTemplateFile
 * This parameter is ignored and should be OFC_HANDLE_NULL
 *
 * \returns
 * If the function fails, the return valid is OFC_INVALID_HANDLE_VALUE
 * If it succeeds, it will return a file handle.
 */
OFC_CORE_LIB OFC_HANDLE
OfcCreateFileW(OFC_LPCTSTR lpFileName,
               OFC_DWORD dwDesiredAccess,
               OFC_DWORD dwShareMode,
               OFC_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
               OFC_DWORD dwCreationDisposition,
               OFC_DWORD dwFlagsAndAttributes,
               OFC_HANDLE hTemplateFile);

/**
 * Create or Open a File Using Normal Characters
 * \see OfcCreateFileW
 */
OFC_CORE_LIB OFC_HANDLE
OfcCreateFileA(OFC_LPCSTR lpFileName,
               OFC_DWORD dwDesiredAccess,
               OFC_DWORD dwShareMode,
               OFC_LPSECURITY_ATTRIBUTES lpSecurityAttributes,
               OFC_DWORD dwCreationDisposition,
               OFC_DWORD dwFlagsAndAttributes,
               OFC_HANDLE hTemplateFile);
/**
 * Create a directory
 *
 * \param lpPathName
 * Path to the directory to create
 *
 * \param lpSecurityAttr
 * Security Attributes for the directory
 *
 * \returns
 * OFC_TRUE if success, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcCreateDirectoryW(OFC_LPCTSTR lpPathName,
                    OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr);

/**
 * Create a Directory Using Normal Characters
 * \see OfcCreateDirectoryW
 */
OFC_CORE_LIB OFC_BOOL
OfcCreateDirectoryA(OFC_LPCSTR lpPathName,
                    OFC_LPSECURITY_ATTRIBUTES lpSecurityAttr);
/**
 * Deletes an existing file
 *
 * \param lpFileName
 * The name of the file in UNC or SMB URL format
 *
 * \returns
 * If success, returns OFC_TRUE, if failure returns OFC_FALSE
 */
OFC_CORE_LIB OFC_BOOL
OfcDeleteFileW(OFC_LPCTSTR lpFileName);

/**
 * Delete a File Using Normal Characters
 * \see OfcDeleteFileW
 */
OFC_CORE_LIB OFC_BOOL
OfcDeleteFileA(OFC_LPCSTR lpFileName);
/**
 * Deletes a directory
 *
 * \param lpPathName
 * The name of the file in UNC or SMB URL format
 *
 * \returns
 * If success, returns OFC_TRUE, if failure returns OFC_FALSE
 */
OFC_CORE_LIB OFC_BOOL
OfcRemoveDirectoryW(OFC_LPCTSTR lpPathName);

/**
 * Delete a Directory Using Normal Characters
 * \see OfcRemoveDirectoryW
 */
OFC_CORE_LIB OFC_BOOL
OfcRemoveDirectoryA(OFC_LPCSTR lpPathName);
/**
 * Searches a directory for a file or subdirectory that matches the name
 * or pattern.
 *
 * \param lpFileName
 * The file name or pattern.  The file name can contain '?' characters which
 * will specified a wildcard for that particular character, or '*' which
 * specifies a wildcard for all characters up to the next text pattern or
 * the next directory level.  For instance File*me would match FileName and
 * FileGnome.  Direct* would match Direct, Directory, DirectDeposit
 *
 * \param lpFindFileData
 * A pointer to the OFC_WIN32_FIND_DATA structure.
 *
 * \param more
 * Pointer to where to return more indication.  OFC_TRUE says more files are
 * available.  OFC_FALSE says this is the last one
 *
 * \returns
 * If the function succeeds, it will return a handle that can be used
 * in subsequent OfcFindNextFile or OfcFindClose call.
 * If the function failed, it will return OFC_INVALID_HANDLE_VALUE.
 */
OFC_CORE_LIB OFC_HANDLE
OfcFindFirstFileW(OFC_LPCTSTR lpFileName,
                  OFC_LPWIN32_FIND_DATAW lpFindFileData,
                  OFC_BOOL *more);

/**
 * 
 * Searches a Directory Using a Normal Character Pattern
 *
 * \see OfcFindFirstFileW
 */
OFC_CORE_LIB OFC_HANDLE
OfcFindFirstFileA(OFC_LPCSTR lpFileName,
                  OFC_LPWIN32_FIND_DATAA lpFindFileData,
                  OFC_BOOL *more);
/**
 * Continues a search from a previous call to OfcFindFirstFile
 *
 * \param hFindFile
 * The search handle returned from OfcFindFirstFile
 *
 * \param lpFindFileData
 * A pointer to the OFC_WIN32_FIND_DATA structure.  
 * \see http://msdn2.microsoft.com/en-us/library/aa365247.aspx
 *
 * \param more
 * Pointer to where to return more indication.  OFC_TRUE says more files are
 * available.  OFC_FALSE says this is the last one
 *
 * \returns
 * OFC_TRUE if the call succeeded, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcFindNextFileW(OFC_HANDLE hFindFile,
                 OFC_LPWIN32_FIND_DATAW lpFindFileData,
                 OFC_BOOL *more);
/**
 * Continue a search using Normal Characters
 *
 * \see OfcFindNextFileW
 */
OFC_CORE_LIB OFC_BOOL
OfcFindNextFileA(OFC_HANDLE hFindFile,
                 OFC_LPWIN32_FIND_DATAA lpFindFileData,
                 OFC_BOOL *more);
/**
 * Closes a file search handle opened by a call to OfcFindFirstFile
 *
 * \param hFindFile
 * The file search handle
 *
 * \returns
 * OFC_TRUE if the call succeeded, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcFindClose(OFC_HANDLE hFindFile);
/**
 * Write all buffered data to the file and clear any buffer cache
 *
 * \param hFile
 * A handle to the open file.
 *
 * \returns
 * OFC_TRUE if the call succeeded, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcFlushFileBuffers(OFC_HANDLE hFile);
/**
 * Retrieves Attributes for a specified file or directory
 *
 * \param lpFileName
 * The name of the file or directory
 *
 * \param fInfoLevelId
 * The information class to retrieve.  Must be OfcGetFileExInfoStandard
 *
 * \param lpFileInformation
 * A pointer to the buffer that received the attribute info.  Must be
 * a pointer to a OFC_WIN32_FILE_ATTRIBUTE_DATA structure
 * see http://msdn2.microsoft.com/en-us/library/aa365739.aspx
 *
 * \returns
 * OFC_TRUE if the function succeeded, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcGetFileAttributesExW(OFC_LPCTSTR lpFileName,
                        OFC_GET_FILEEX_INFO_LEVELS fInfoLevelId,
                        OFC_LPVOID lpFileInformation);
/**
 * Retrieves File Attributes Using Normal Characters
 *
 * \see OfcGetFileAttriubtesExW
 */
OFC_CORE_LIB OFC_BOOL
OfcGetFileAttributesExA(OFC_LPCSTR lpFileName,
                        OFC_GET_FILEEX_INFO_LEVELS fInfoLevelId,
                        OFC_LPVOID lpFileInformation);
/**
 * Retrieves Attribute Info for a file by File Handle
 *
 * \param hFile
 * Handle to the file
 *
 * \param FileInformationClass
 * Value that specifies the type of info to return
 *
 * \param lpFileInformation
 * Pointer to buffer that receives the requested file informaiton
 * OFC_FILE_BASIC_INFO
 * OFC_FILE_STANDARD_INFO
 * OFC_FILE_NAME_INFO
 * OFC_FILE_STREAM_INFO
 * OFC_FILE_COMPRESSION_INFO
 * OFC_FILE_ATTRIBUTE_TAG_INFO
 * OFC_FILE_ID_BOTH_DIR_INFO
 *
 * \param dwBufferSize
 * Size of the lpFileInformation buffer
 *
 * \returns
 * OFC_TRUE if the call succeeded, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcGetFileInformationByHandleEx(OFC_HANDLE hFile,
                                OFC_FILE_INFO_BY_HANDLE_CLASS
                                FileInformationClass,
                                OFC_LPVOID lpFileInformation,
                                OFC_DWORD dwBufferSize);

/**
 * Moves an existing file or directory
 *
 * \param lpExistingFileName
 * The current name of the file or directory
 *
 * \param lpNewFileName
 * The new name for the file or directory
 *
 * \returns
 * OFC_TRUE if success, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcMoveFileW(OFC_LPCTSTR lpExistingFileName,
             OFC_LPCTSTR lpNewFileName);
/**
 * Moves a file (i.e Rename) Using Normal Characters
 *
 * \see OfcMoveFileW
 */
OFC_CORE_LIB OFC_BOOL
OfcMoveFileA(OFC_LPCSTR lpExistingFileName,
             OFC_LPCSTR lpNewFileName);
/**
 * Reads data from a file
 *
 * \param hFile
 * A Handle to the file to be read
 *
 * \param lpBuffer
 * A pointer to the buffer that receives the data read from a file
 *
 * \param nNumberOfBytesToRead
 * The Maximum numer of bytes to be read
 *
 * \param lpNumberOfBytesRead
 * A pointer to a variable to receive the number of bytes read
 *
 * \param hOverlapped
 * Handle to the overlapped structure
 *
 * \returns
 * OFC_FALSE if the function fails, OFC_TRUE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcReadFile(OFC_HANDLE hFile,
            OFC_LPVOID lpBuffer,
            OFC_DWORD nNumberOfBytesToRead,
            OFC_LPDWORD lpNumberOfBytesRead,
            OFC_HANDLE hOverlapped);
/**
 * Create an Overlapped I/O Structure for the desired platform
 *
 * This is used by Read and Write calls to schedule overlapped I/O.
 * The implementation of Overlapped I/O is platform specific so the
 * context for an overlapped I/O is also platform specific
 *
 * \returns
 * Handle to the Overlapped I/O Context
 */
OFC_CORE_LIB OFC_HANDLE
OfcCreateOverlapped(OFC_HANDLE hFile);
/**
 * Destroy an Overlapped I/O context
 *
 * \param hFile
 * File handle that overlapped context handle is for
 *
 * \param hOverlapped
 * The overlapped context to destroy
 */
OFC_CORE_LIB OFC_VOID
OfcDestroyOverlapped(OFC_HANDLE hFile, OFC_HANDLE hOverlapped);
/**
 * Set the offset at which an overlapped I/O should occur
 *
 * Since Overlapped I/O is platform specific, this abstraction
 * is provided to communicate to the File handler the file offset
 * to perform the I/O at
 *
 * \param hFile
 * File to set the offset for
 *
 * \param hOverlapped
 * The handle to the overlapped context
 *
 * \param offset
 * The file offset to set
 */
OFC_CORE_LIB OFC_VOID
OfcSetOverlappedOffset(OFC_HANDLE hFile, OFC_HANDLE hOverlapped,
                       OFC_OFFT offset);
/**
 * Retrieves the results of an overlapped operation on the specified file
 *
 * \param hFile
 * A handle to the file
 *
 * \param hOverlapped
 * Handle to the overlapped structure
 *
 * \param lpNumberOfBytesTransferred
 * Number of bytes read or written during the operation
 *
 * \param bWait
 * Whether this function should block until the operation completes.  If this
 * is false, and the operation is not complete, the call returns false.
 *
 * \returns
 * OFC_TRUE if the function succeeds, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcGetOverlappedResult(OFC_HANDLE hFile,
                       OFC_HANDLE hOverlapped,
                       OFC_LPDWORD lpNumberOfBytesTransferred,
                       OFC_BOOL bWait);
/**
 * Sets the physical file size for the specified file to the current position
 *
 * \param hFile
 * Handle to the file
 *
 * \returns
 * OFC_TRUE if success, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcSetEndOfFile(OFC_HANDLE hFile);
/**
 * Sets the attributes for a file or directory
 *
 * \param lpFileName
 * Name of the file
 *
 * \param dwFileAttributes
 * The file attributes to set for the file
 *
 * \returns
 * OFC_TRUE if success, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcSetFileAttributesW(OFC_LPCTSTR lpFileName,
                      OFC_DWORD dwFileAttributes);
/**
 * Set File Attributes Using Normal Characters
 *
 * \see OfcSetFileAttributesW
 */
OFC_CORE_LIB OFC_BOOL
OfcSetFileAttributesA(OFC_LPCSTR lpFileName,
                      OFC_DWORD dwFileAttributes);
/**
 * Set the file information for the specified file
 *
 * \param hFile
 * Handle to the file
 *
 * \param FileInformationClass
 * An enumeration that specifies the type of information to be changed
 *
 * \param lpFileInformation
 * A pointer to the buffer that contains the information to be set
 * OFC_FILE_BASIC_INFO, OFC_FILE_RENAME_INFO, OFC_FILE_DISPOSITION_INFO,
 * OFC_FILE_ALLOCATION_INFO, OFC_FILE_END_OF_FILE_INFO,
 * OFC_FILE_IO_PRIORITY_HINT_INFO
 *
 * \param dwBufferSize
 * The size of the lpFileInformation
 *
 * \returns
 * OFC_TRUE if success, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcSetFileInformationByHandle(OFC_HANDLE hFile,
                              OFC_FILE_INFO_BY_HANDLE_CLASS
                              FileInformationClass,
                              OFC_LPVOID lpFileInformation,
                              OFC_DWORD dwBufferSize);
/**
 * Moves the file pointer of the specified file
 *
 * \param hFile
 * File Handle
 *
 * \param lDistanceToMove
 * The lower order 32 bits of a signed value that specifies the number
 * of bytes to move relative to the move method
 *
 * \param lpDistanceToMoveHigh
 *  pointer to the high order 32 bits.  This may be NULL.  If not NULL
 * it also returns the high order 32 bits of the resulting pointer.
 * (the function's return value returns the low order 32 bits)
 *
 * \param dwMoveMethod
 * The starting point for the file pointer move
 * (FILE_BEGIN, FILE_CURRENT, FILE_END)
 *
 * \returns
 * OFC_INVALID_SET_FILE_POINTER if Failed, low order file pointer
 * if success.  Since OFC_INVALID_SET_FILE_POINTER
 * may also be interpreted as a valid low order file pointer, care should
 * be taken not to get false negatives.
 */
OFC_CORE_LIB OFC_DWORD
OfcSetFilePointer(OFC_HANDLE hFile, OFC_LONG lDistanceToMove,
                  OFC_PLONG lpDistanceToMoveHigh,
                  OFC_DWORD dwMoveMethod);

/**
 * Writes Data to the specified file
 *
 * \param hFile
 * Handle to the file
 *
 * \param lpBuffer
 * Pointer to buffer containing data to write
 *
 * \param nNumberOfBytesToWrite
 * The number of bytes to write
 *
 * \param lpNumberOfBytesWritten
 * A pointer to a long to contain number of bytes written
 *
 * \param hOverlapped
 * Handle to the overlapped structure for asynchronous I/O
 *
 * \returns
 * OFC_TRUE if success, OFC_FALSE if failed
 */
OFC_CORE_LIB OFC_BOOL
OfcWriteFile(OFC_HANDLE hFile,
             OFC_LPCVOID lpBuffer,
             OFC_DWORD nNumberOfBytesToWrite,
             OFC_LPDWORD lpNumberOfBytesWritten,
             OFC_HANDLE hOverlapped);
/**
 * Perform a transaction on a named pipe
 *
 * \param hFile
 * File handle of named pipe
 *
 * \param lpInBuffer
 * Pointer to the buffer to send to the pipe
 *
 * \param nInBufferSize
 * Size of the input buffer
 *
 * \param lpOutBuffer
 * Pointer to the buffer to receive the response
 *
 * \param nOutBufferSize
 * Size of the output buffer
 *
 * \param lpBytesRead
 * Pointer to where to return the number of bytes read
 *
 * \param hOverlapped
 * Handle to the overlapped context if this is asynchronous
 *
 * \returns
 * OFC_TRUE if success, OFC_FALSE if failure.  On failure, GetLastError
 * will return the error code
 */
OFC_CORE_LIB OFC_BOOL
OfcTransactNamedPipe(OFC_HANDLE hFile,
                     OFC_LPVOID lpInBuffer,
                     OFC_DWORD nInBufferSize,
                     OFC_LPVOID lpOutBuffer,
                     OFC_DWORD nOutBufferSize,
                     OFC_LPDWORD lpBytesRead,
                     OFC_HANDLE hOverlapped);
/**
 * Get the last file error that occured in this file handler for
 * this thread
 *
 * \param hHandle
 * Handle of File
 *
 * \returns
 * Last File Error
 *
 * NOTE: Use OfcGetLastError instead.
 */
OFC_CORE_LIB OFC_UINT32
OfcGetLastFileError(OFC_HANDLE hHandle);
/**
 * Get the last error encountered by the File Component of
 * Open Files
 *
 * \returns
 * Error Code
 */
OFC_CORE_LIB OFC_DWORD
OfcGetLastError(OFC_VOID);
/**
 * Get Error String from Error
 *
 * \param dwerr
 * Error Code
 *
 * \returns
 * Error String
 */
OFC_CORE_LIB const OFC_CHAR *ofc_get_error_string(OFC_DWORD dwerr);
/**
 * Get the last file error that occured in the file handler for a
 * specified file in this thread's context
 *
 * \param lpFileName
 * Name of the file to map to a file handler
 *
 * \returns
 * Last File Error
 *
 * NOTE: Use OfcGetLastError instead.
 */
OFC_CORE_LIB OFC_UINT32
OfcGetLastFileNameErrorW(OFC_LPCTSTR lpFileName);
/**
 * Get Last Error for a File Using Normal Characters
 *
 * \see OfcGetLastFileNameErrorW
 */
OFC_CORE_LIB OFC_UINT32
OfcGetLastFileNameErrorA(OFC_LPCSTR lpFileName);
/**
 * Get the amount of free space for the disk
 *
 * \param lpRootPathName
 * The path name to the disk or share
 *
 * \param lpSectorsPerCluster
 * Pointer to where to return the number of sectors per cluster for the
 * volume
 *
 * \param lpBytesPerSector
 * Pointer to where to return the number of bytes per sector for the
 * volume
 *
 * \param lpNumberOfFreeClusters
 * Pointer to where to return the number of free clusters
 *
 * \param lpTotalNumberOfClusters
 * Pointer to where to return the total number of clusters
 *
 * \returns
 * OFC_TRUE if success, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcGetDiskFreeSpaceW(OFC_LPCTSTR lpRootPathName,
                     OFC_LPDWORD lpSectorsPerCluster,
                     OFC_LPDWORD lpBytesPerSector,
                     OFC_LPDWORD lpNumberOfFreeClusters,
                     OFC_LPDWORD lpTotalNumberOfClusters);
/**
 * Get Disk Free Space Using Normal Characters
 *
 * \see OfcGetDiskFreeSpaceW
 */
OFC_CORE_LIB OFC_BOOL
OfcGetDiskFreeSpaceA(OFC_LPCSTR lpRootPathName,
                     OFC_LPDWORD lpSectorsPerCluster,
                     OFC_LPDWORD lpBytesPerSector,
                     OFC_LPDWORD lpNumberOfFreeClusters,
                     OFC_LPDWORD lpTotalNumberOfClusters);
/**
 * Get Volume Information for the volume
 *
 * \param lpRootPathName
 * Path to the volume or share
 *
 * \param lpVolumeNameBuffer
 * Pointer to buffer where to store the Volume Name
 *
 * \param nVolumeNameSize
 * Size of the Volume Name Buffer
 *
 * \param lpVolumeSerialNumber
 * Pointer to where to store the volume serial number
 *
 * \param lpMaximumComponentLength
 * Pointer to where to store the max size of a file name path
 *
 * \param lpFileSystemFlags
 * Pointer to where to store the file system flags
 *
 * \param lpFileSystemName
 * Pointer to where to store the file system name (i.e. NTFS)
 *
 * \param nFileSystemName
 * Size of the file system name buffer
 *
 * \returns
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcGetVolumeInformationW(OFC_LPCTSTR lpRootPathName,
                         OFC_LPTSTR lpVolumeNameBuffer,
                         OFC_DWORD nVolumeNameSize,
                         OFC_LPDWORD lpVolumeSerialNumber,
                         OFC_LPDWORD lpMaximumComponentLength,
                         OFC_LPDWORD lpFileSystemFlags,
                         OFC_LPTSTR lpFileSystemName,
                         OFC_DWORD nFileSystemName);
/**
 * Get Volume Information Using Normal Characters
 *
 * \see OfcGetVolumeInformationW
 */
OFC_CORE_LIB OFC_BOOL
OfcGetVolumeInformationA(OFC_LPCSTR lpRootPathName,
                         OFC_LPSTR lpVolumeNameBuffer,
                         OFC_DWORD nVolumeNameSize,
                         OFC_LPDWORD lpVolumeSerialNumber,
                         OFC_LPDWORD lpMaximumComponentLength,
                         OFC_LPDWORD lpFileSystemFlags,
                         OFC_LPSTR lpFileSystemName,
                         OFC_DWORD nFileSystemName);
/**
 * Return the type of file system that a file is on
 *
 * \param hHandle
 * Handle to the file
 *
 * \returns
 * The type of file system
 */
OFC_CORE_LIB OFC_FST_TYPE
OfcFileGetFSType(OFC_HANDLE hHandle);
/**
 * An internal call to return the native file system handle
 *
 * \param hHandle
 * Handle to the file
 *
 * \returns
 * A handle to the lower file system specific file
 */
OFC_CORE_LIB OFC_HANDLE
OfcFileGetFSHandle(OFC_HANDLE hHandle);
/**
 * Unlock a region in a file
 *
 * \param hFile
 * File Handle to unlock
 *
 * \param length_low
 * the low order 32 bits of the length of the region
 *
 * \param length_high
 * the high order 32 bits of the length of the region
 *
 * \param hOverlapped
 * The overlapped structure which specifies the offset
 *
 * \returns
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcUnlockFileEx(OFC_HANDLE hFile,
                OFC_UINT32 length_low,
                OFC_UINT32 length_high,
                OFC_HANDLE hOverlapped);
/**
 * Lock a region of a file
 *
 * \param hFile
 * Handle to file to unlock region in
 *
 * \param flags
 * Flags for lock
 *
 * \param length_low
 * Low order 32 bits of length of region
 *
 * \param length_high
 * High order 32 bits of length of region
 *
 * \param hOverlapped
 * Handle to the overlapped structure
 *
 * \returns
 * OFC_TRUE if successful, OFC_FALSE otherwise
 */
OFC_CORE_LIB OFC_BOOL
OfcLockFileEx(OFC_HANDLE hFile, OFC_DWORD flags,
              OFC_DWORD length_low, OFC_DWORD length_high,
              OFC_HANDLE hOverlapped);
/**
 * Get the event handle of the overlapped event (deprecated)
 *
 * \param hOverlapped
 * The overlapped event
 *
 * \returns
 * The event handle
 */
OFC_CORE_LIB OFC_HANDLE
OfcFileGetOverlappedEvent(OFC_HANDLE hOverlapped);
/**
 * Get the Message Queue of the overlapped event
 *
 * \param hOverlapped
 * The overlapped event
 *
 * \returns
 * The msgq handle
 */
OFC_CORE_LIB OFC_HANDLE
OfcFileGetOverlappedWaitQ(OFC_HANDLE hOverlapped);
/**
 * Dismount an SMB client connection
 *
 * This is a non-standard and optional API for those that wish to
 * ensure that a client connection to a server is disconnected.
 * Under normal operation, client connections are disconnected after
 * idle for a period of time.
 *
 * \param lpFileName
 * A path that includes the server URL
 *
 * \returns
 * Status
 */
OFC_CORE_LIB OFC_BOOL
OfcDismountW(OFC_LPCTSTR lpFileName);
/**
 * Dismount a File Using Normal Characters
 *
 * \see OfcDismountW
 */
OFC_CORE_LIB OFC_BOOL
OfcDismountA(OFC_LPCSTR lpFileName);
/**
 * Perform a Device IOCTL
 *
 * This is an internal API that is used for DFS resolution
 */
OFC_CORE_LIB OFC_BOOL OfcDeviceIoControl(OFC_HANDLE hFile,
                                         OFC_DWORD dwIoControlCode,
                                         OFC_LPVOID lpInBuffer,
                                         OFC_DWORD nInBufferSize,
                                         OFC_LPVOID lpOutBuffer,
                                         OFC_DWORD nOutBufferSize,
                                         OFC_LPDWORD lpBytesReturned,
                                         OFC_HANDLE hOverlapped);

/**
 * Set the Current Directory
 * 
 * This will set the default directory for relative file paths
 *
 * \param lpPathName
 * A path that includes the server URL
 *
 * \returns
 * Status. Non-zero if succeeds, zero if failed.  (See OfcGetLasstError)
 */
OFC_CORE_LIB OFC_BOOL OfcSetCurrentDirectoryW(OFC_LPCTSTR lpPathName);
OFC_CORE_LIB OFC_BOOL OfcSetCurrentDirectoryA(OFC_LPCSTR lpPathName);
/**
 * Get the Current Directory
 * 
 * This will return the default directory for relative file paths
 *
 * \param nBufferLength
 * Size of buffer to return the current directory to
 * 
 * \param lpBuffer
 * Buffer to write the current directory to
 *
 * \returns
 * Number of characters in the buffer (not including terminating char) or zero
 * if it failed (See OfcGetLastError)
 */
OFC_CORE_LIB OFC_DWORD OfcGetCurrentDirectoryW(OFC_DWORD nBufferLength,
					       OFC_LPTSTR lpBuffer);
OFC_CORE_LIB OFC_DWORD OfcGetCurrentDirectoryA(OFC_DWORD nBufferLength,
					       OFC_LPSTR lpBuffer);


#if defined(__cplusplus)
}
#endif

#if defined(OFC_UNICODE_API)
/**
 * Create or open a file using default character encoding
 *
 * \see OfcCreateFileW
 * \see OfcCreateFileA
 */
#define OfcCreateFile OfcCreateFileW
/**
 * Create a directory using default character encoding
 *
 * \see OfcCreateDirectoryW
 * \see OfcCreateDirectoryA
 */
#define OfcCreateDirectory OfcCreateDirectoryW
/**
 * Delete a file using default character encoding
 *
 * \see OfcDeleteFileW
 * \see OfcDeleteFileA
 */
#define OfcDeleteFile OfcDeleteFileW
/**
 * Remove a directory using default character encoding
 *
 * \see OfcRemoveDirectoryW
 * \see OfcRemoveDirectoryA
 */
#define OfcRemoveDirectory OfcRemoveDirectoryW
/**
 * Start a file search using default character encoding
 *
 * \see OfcFindFirstW
 * \see OfcFindFirstA
 */
#define OfcFindFirstFile OfcFindFirstFileW
/**
 * Return the next file in search using default character encoding
 *
 * \see OfcFindNextFileW
 * \see OfcFindNextFileA
 */
#define OfcFindNextFile OfcFindNextFileW
/**
 * Get file attributes using default file encoding
 * 
 * \see OfcGetFileAttributesExW
 * \see OfcGetFileAttributesExA
 */
#define OfcGetFileAttributesEx OfcGetFileAttributesExW
/**
 * Move a file using default chracter encoding
 *
 * \see OfcMoveFileW
 * \see OfcMoveFileA
 */
#define OfcMoveFile OfcMoveFileW
/**
 * Set file attributes using default character encoding
 *
 * \see OfcSetFileAttributesW
 * \see OfcSetFileAttributesA
 */
#define OfcSetFileAttributes OfcSetFileAttributesW
/**
 * Get last error on file name using default character encoding
 *
 * \see OfcGetLastFileNameErrorW
 * \see OfcGetLastFileNameErrorA
 */
#define OfcGetLastFileNameError OfcGetLastFileNameErrorW
/**
 * Get disk free space using default character encoding
 *
 * \see OfcGetDiskFreeSpaceW
 * \see OfcGetDiskFreeSpaceA
 */
#define OfcGetDiskFreeSpace OfcGetDiskFreeSpaceW
/**
 * Get volumeinformation using default character encoding
 *
 * \see OfcGetVolumeInformationW
 * \see OfcGetVolumeInformationA
 */
#define OfcGetVolumeInformation OfcGetVolumeInformationW
/**
 * Dismount share using default default character encoding
 *
 * \see OfcDismountW
 * \see OfcDismountA
 */
#define OfcDismount OfcDismountW
/**
 * Set the Current Directory
 *
 * \see OfcSetCurrentDirectoryW
 * \see OfcSetCurrentDirectoryA
 */
#define OfcSetCurrentDirectory OfcSetCurrentDirectoryW
/**
 * Get the Current Directory
 *
 * \see OfcGetCurrentDirectoryW
 * \see OfcGetCurrentDirectoryA
 */
#define OfcGetCurrentDirectory OfcGetCurrentDirectoryW
/**
 * \see OFC_WIN32_FIND_DATAW
 * \see OFC_WIN32_FIND_DATAA
 */
#define OFC_WIN32_FIND_DATA OFC_WIN32_FIND_DATAW
/**
 * \see OFC_LPWIN32_FIND_DATAW
 * \see OFC_LPWIN32_FIND_DATAA
 */
#define OFC_LPWIN32_FIND_DATA OFC_LPWIN32_FIND_DATAW

#else
#define OfcCreateFile OfcCreateFileA
#define OfcCreateDirectory OfcCreateDirectoryA
#define OfcDeleteFile OfcDeleteFileA
#define OfcRemoveDirectory OfcRemoveDirectoryA
#define OfcFindFirstFile OfcFindFirstFileA
#define OfcFindNextFile OfcFindNextFileA
#define OfcGetFileAttributesEx OfcGetFileAttributesExA
#define OfcMoveFile OfcMoveFileA
#define OfcSetFileAttributes OfcSetFileAttributesA
#define OfcGetLastFileNameError OfcGetLastFileNameErrorA
#define OfcGetDiskFreeSpace OfcGetDiskFreeSpaceA
#define OfcGetVolumeInformation OfcGetVolumeInformationA
#define OfcDismount OfcDismountA
#define OfcSetCurrentDirectory OfcSetCurrentDirectoryA
#define OfcGetCurrentDirectory OfcGetCurrentDirectoryA

#define OFC_WIN32_FIND_DATA OFC_WIN32_FIND_DATAA
#define OFC_LPWIN32_FIND_DATA OFC_LPWIN32_FIND_DATAA

#endif
/** \} */
#endif

