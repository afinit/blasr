// Author: Yuan Li

#ifndef _BAM2BAX_METADATA_WRITER_H_
#define _BAM2BAX_METADATA_WRITER_H_

#include <iostream>
#include <fstream>
#include <cassert>
#include <pbbam/ReadGroupInfo.h>

namespace internal{

const std::string DEFAULT_ANALYSIS_DIR = "Analysis_Results";

const std::string META_CONTENT = 
"<?xml version=\"1.0\" encoding=\"utf-8\"?><Metadata xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"http://pacificbiosciences.com/PAP/Metadata.xsd\"><InstCtrlVer>__BASECALLERVERSION__</InstCtrlVer><CellIndex>3</CellIndex><SetNumber>1</SetNumber><BindingKit><PartNumber>__BINDINGKIT__</PartNumber></BindingKit><SequencingKit><PartNumber>__SEQUENCINGKIT__</PartNumber></SequencingKit><Primary><Protocol>BasecallerV1</Protocol><ResultsFolder>__ANALYSISDIR__</ResultsFolder></Primary></Metadata>";

std::string Replace(const std::string & in_str,
                    const std::string & to_find,
                    const std::string & to_replace);
} //namespace internal

class MetadataWriter {
public: 
    MetadataWriter(const std::string & filename, 
                   const PacBio::BAM::ReadGroupInfo & rg,
                   const std::string & analysisDir=internal::DEFAULT_ANALYSIS_DIR);

    MetadataWriter(const std::string & filename, 
                   const std::string & basecallerVersion,
                   const std::string & sequencingKit,
                   const std::string & bindingKit,
                   const std::string & analysisDir);

    ~MetadataWriter(void) {}
};

#endif
