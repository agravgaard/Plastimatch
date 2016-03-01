/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "itkImageRegionIterator.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"

#include "itk_metadata.h"

void 
itk_metadata_set (
    itk::MetaDataDictionary *dict, 
    const char *tag, 
    const char *value
)
{
    typedef itk::MetaDataObject< std::string > MetaDataStringType;
    itk::EncapsulateMetaData<std::string> (
	*dict, std::string (tag), std::string (value));

    itk::MetaDataDictionary::ConstIterator itr = dict->Begin();
    itk::MetaDataDictionary::ConstIterator end = dict->End();

    while ( itr != end ) {
	itk::MetaDataObjectBase::Pointer entry = itr->second;
	MetaDataStringType::Pointer entryvalue =
	    dynamic_cast<MetaDataStringType *>( entry.GetPointer());
	if (entryvalue) {
	    std::string tagkey = itr->first;
	    std::string tagvalue = entryvalue->GetMetaDataObjectValue();
	    std::cout << tagkey << " = " << tagvalue << std::endl;
	}
	++itr;
    }
}

void 
itk_metadata_print_1 (
    itk::MetaDataDictionary *dict
)
{
    typedef itk::MetaDataObject< std::string > MetaDataStringType;

    itk::MetaDataDictionary::ConstIterator itr = dict->Begin();
    itk::MetaDataDictionary::ConstIterator end = dict->End();

    printf ("ITK Metadata...\n");
    while ( itr != end ) {
	itk::MetaDataObjectBase::Pointer entry = itr->second;
	MetaDataStringType::Pointer entryvalue =
	    dynamic_cast<MetaDataStringType *>( entry.GetPointer());
	if (entryvalue) {
	    std::string tagkey = itr->first;
	    std::string tagvalue = entryvalue->GetMetaDataObjectValue();
	    std::cout << tagkey << " = " << tagvalue << std::endl;
	}
	++itr;
    }
}

/* This is just another example of how to use the API */
void 
itk_metadata_print_2 (
    itk::MetaDataDictionary *dict
)
{
    typedef itk::MetaDataObject< std::string > MetaDataStringType;

    std::vector<std::string> keys = dict->GetKeys();
    std::vector<std::string>::const_iterator key = keys.begin();

    std::string meta_string;

    printf ("ITK Metadata (2)...\n");
    while (key != keys.end()) {
	std::cout << *key << " " << meta_string << std::endl;
	++key;
    }
}

void 
itk_metadata_print (
    itk::MetaDataDictionary *dict
)
{
    itk_metadata_print_1 (dict);
}
