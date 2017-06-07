#include "buffer.hpp"
#include "string.hpp"
#include "checksum.hpp"

namespace mush
{
    namespace zip
    {
        /*
            4.3.6 Overall .ZIP file format:

              [local file header 1]
              [encryption header 1]
              [file data 1]
              [data descriptor 1]
              . 
              .
              .
              [local file header n]
              [encryption header n]
              [file data n]
              [data descriptor n]
              [archive decryption header] 
              [archive extra data record] 
              [central directory header 1]
              .
              .
              .
              [central directory header n]
              [zip64 end of central directory record]
              [zip64 end of central directory locator] 
              [end of central directory record]
        */
        constexpr uint16_t  HAS_DATA_DESCRIPTOR_RECORD = 0b10;

        struct LocalFileHeader
        {
            uint32_t    local_file_header_signature; // (0x04034b50)
            uint16_t    version_needed_to_extract;
            uint16_t    general_purpose_bit_flag;
            uint16_t    compression_method;
            uint16_t    last_mod_file_time;
            uint16_t    last_mod_file_date;
            uint32_t    crc32;
            uint32_t    compressed_size;
            uint32_t    uncompressed_size;
            uint16_t    filename_length;
            uint16_t    extra_field_length;

            string      filename;
            Buffer      extra_field;
        };

        struct DataDescriptor
        {
            uint32_t    crc32;
            uint32_t    compressed_size;
            uint32_t    uncompressed_size;
        };

        struct ArchiveExtraData
        {
            uint32_t    archive_extra_data_signature;   // (0x08064b50)
            uint32_t    extra_field_length;
            Buffer      extra_field_data;
        };
        /*
        4.3.12  Central directory structure:

          [central directory header 1]
          .
          .
          . 
          [central directory header n]
          [digital signature] 

          File header:

            central file header signature   4 bytes  (0x02014b50)
            version made by                 2 bytes
            version needed to extract       2 bytes
            general purpose bit flag        2 bytes
            compression method              2 bytes
            last mod file time              2 bytes
            last mod file date              2 bytes
            crc-32                          4 bytes
            compressed size                 4 bytes
            uncompressed size               4 bytes
            file name length                2 bytes
            extra field length              2 bytes
            file comment length             2 bytes
            disk number start               2 bytes
            internal file attributes        2 bytes
            external file attributes        4 bytes
            relative offset of local header 4 bytes

            file name (variable size)
            extra field (variable size)
            file comment (variable size)
        */
        
        struct CentralFileHeader
        {
            uint32_t    central_file_header_signature; // (0x02014b50)
            uint16_t    version_made_by;
            uint16_t    version_needed_to_extract;
            uint16_t    general_purpose_bit_flag;
            uint16_t    compression_method;
            uint16_t    last_mod_file_time;
            uint16_t    last_mod_file_date;
            uint32_t    crc32;
            uint32_t    compressed_size;
            uint32_t    uncompressed_size;
            uint16_t    file_name_length;
            uint16_t    extra_field_length;
            uint16_t    disk_number_start;
            uint16_t    internal_file_attributes;
            uint32_t    external_file_attributes;
            uint32_t    relative_offset_of_local_header;

            string      file_name;
            Buffer      extra_field;
            Buffer      file_comment;
        };

        struct DigitalSignature
        {
            uint32_t    header_signature; // (0x05054b50)
            uint16_t    size_of_data;
            Buffer      signature_data;
        };

        struct Zip64ExtensibleDataBlock
        {
            uint16_t    header_id;
            uint32_t    data_size;

            Buffer      data;
        };

        struct Zip64EndOfCentralDirectory
        {
            uint32_t    signature; // (0x06065b50)
            uint64_t    size_of_record; // size of remaining record, ignore leading 12 bytes
            uint16_t    version_made_by;
            uint16_t    version_needed_to_extract;
            uint32_t    number_of_this_disk;
            uint32_t    central_directory_start_disk;
            uint64_t    total_number_of_entries_this_disk;
            uint64_t    total_number_of_entries;
            uint64_t    size_of_central_directory;
            uint64_t    offset_to_disk_number;

            std::vector<Zip64ExtensibleDataBlock> extensible_data_sector;
        };

        struct Zip64EndOfCentralDirectoryLocator
        {
            uint32_t    signature; // (0x07054b50)
            uint32_t    central_directory_end_start_disk;
            uint64_t    relative_offset;
            uint32_t    total_number_of_disks;
        };

        struct EndOfCentralDirectory
        {
            uint32_t    signature; // (0x0605b450)
            uint16_t    number_of_this_disk;
            uint16_t    central_directory_start_disk;
            uint16_t    total_number_of_entries_this_disk;
            uint16_t    total_number_of_entries;
            uint32_t    size_of_central_directory;
            uint32_t    offset_to_disk_number;
            uint16_t    zip_file_comment_length;

            Buffer      zip_file_comment;
        };
    }

    class ZipFile
    {
    };
}
