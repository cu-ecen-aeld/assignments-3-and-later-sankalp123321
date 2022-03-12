/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/printk.h>
#else
#include <string.h>
#include <stdio.h>
#endif

#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer. 
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
			size_t char_offset, size_t *entry_offset_byte_rtn )
{
    /**
    * TODO: implement per description
    */
    if(buffer == NULL) return NULL;
    
    if(entry_offset_byte_rtn == NULL) return NULL;

    uint8_t totalElementsInTheList = 0;
    if(buffer->in_offs == buffer->out_offs)
    {
        if(buffer->full)
        {
            totalElementsInTheList = AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
        }
        else
        {
            totalElementsInTheList = 0;
        }
    }
    else
    {
        if (buffer->in_offs > buffer->out_offs) 
        {
            totalElementsInTheList = buffer->in_offs - buffer->out_offs + 1;
        }
        else
        {
            totalElementsInTheList = buffer->out_offs - buffer->in_offs + 1;
        }
    }
    for (uint16_t i = buffer->out_offs; i < totalElementsInTheList;)
    {
        if((char_offset < buffer->entry[i].size) || !char_offset)
        {
            *entry_offset_byte_rtn = char_offset;
	        return &buffer->entry[i];
        }
        char_offset = char_offset - buffer->entry[i].size;
	    i = (i+1);
	    if(i == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
        {
            totalElementsInTheList = buffer->out_offs;
	        i = 0;
        }
    }
    *entry_offset_byte_rtn = char_offset; 
    return NULL;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
struct aesd_buffer_entry aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description 
    */
    struct aesd_buffer_entry ret = 
    { 
        .buffptr = NULL, 
        .size= 0
    };
    if (buffer == NULL) return;

    if (add_entry == NULL) return;
    
    if(buffer->full == true)
    {
        ret = buffer->entry[buffer->out_offs];
        buffer->out_offs++;
        buffer->full = false;
    }
    buffer->entry[buffer->in_offs] = *add_entry;
    printk(KERNEL_INFO "New element %s %d are added\n", buffer->entry[buffer->in_offs].buffptr, buffer->entry[buffer->in_offs].size);
    buffer->in_offs = (buffer->in_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    if(buffer->in_offs == buffer->out_offs)
    {
        buffer->full = true;
    }
    return ret;
}
    


/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
