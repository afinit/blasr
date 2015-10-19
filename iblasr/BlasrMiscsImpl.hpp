// Author: Mark Chaisson
#pragma once

#include <utils/SMRTTitle.hpp>

template<typename T_Sequence>
bool GetNextReadThroughSemaphore(ReaderAgglomerate &reader,
                                 MappingParameters &params,
                                 T_Sequence &read,
                                 string & readGroupId,
                                 int & associatedRandInt,
                                 MappingSemaphores & semaphores)
{
    // Wait on a semaphore
    if (params.nProc > 1) {
#ifdef __APPLE__
        sem_wait(semaphores.reader);
#else
        sem_wait(&semaphores.reader);
#endif
    }

    bool returnValue = true;
    //
    // CCS Reads are read differently from other reads.  Do static casting here
    // of this.
    //
    if (reader.GetNext(read, associatedRandInt) == 0) {
        returnValue = false;
    }

    //
    // Set the read group id before releasing the semaphore, since other
    // threads may change the reader object to a new read group before
    // sending this alignment out to printing.
    readGroupId = reader.readGroupId;

    if (params.nProc > 1) {
#ifdef __APPLE__
        sem_post(semaphores.reader);
#else
        sem_post(&semaphores.reader);
#endif
    }
    return returnValue;
}



bool ReadHasMeaningfulQualityValues(FASTQSequence &sequence)
{
    if (sequence.qual.Empty() == true) {
        return 0;
    }
    else {
        int numZero=0, numNonZero=0;
        if (sequence.qual.data == NULL) {
            return false;
        }
        numZero = CountZero(sequence.qual.data, sequence.length);
        numNonZero = sequence.length - numZero;
        int subNumZero = 0, subNonZero = 0;

        if (sequence.substitutionQV.data == NULL) {
            return false;
        }
        subNumZero = CountZero(sequence.substitutionQV.data, sequence.length);
        subNonZero = sequence.length - subNumZero;

        if (numZero < 0.5*numNonZero and subNumZero < 0.5 * subNonZero) {
            return true;
        }
        else {
            return false;
        }
    }
}

// Given a SMRT sequence and a subread interval, make the subread.
// Input:
//   smrtRead         - a SMRT sequence
//   subreadInterval  - a subread interval
//   params           - mapping parameters
// Output:
//   subreadSequence - the constructed subread
void MakeSubreadOfInterval(SMRTSequence & subreadSequence,
                           SMRTSequence & smrtRead,
                           ReadInterval & subreadInterval,
                           MappingParameters & params)
{
    int start = subreadInterval.start;
    int end   = subreadInterval.end;

    assert(smrtRead.length >= subreadSequence.length);
    smrtRead.MakeSubreadAsMasked(subreadSequence, start, end);

    if (!params.preserveReadTitle) {
        smrtRead.SetSubreadTitle(subreadSequence,
                subreadSequence.SubreadStart(),
                subreadSequence.SubreadEnd());
    }
    else {
        subreadSequence.CopyTitle(smrtRead.title);
    }
    subreadSequence.zmwData = smrtRead.zmwData;
}

// Given a SMRT sequence and one of its subreads, make the
// reverse complement of the subread in the coordinate of the
// reverse complement sequence of the SMRT sequence.
// Input:
//   smrtRead          - a SMRT read
//   subreadSequence   - a subread of smrtRead
// Output:
//   subreadSequenceRC - the reverse complement of the subread
//                       in the coordinate of the reverse
//                       complement of the SMRT read.
void MakeSubreadRC(SMRTSequence & subreadSequenceRC,
                   SMRTSequence & subreadSequence,
                   SMRTSequence & smrtRead)
{
    assert(smrtRead.length >= subreadSequence.length);
    // Reverse complement sequence of the subread.
    subreadSequence.MakeRC(subreadSequenceRC);
    // Update start and end positions of subreadSequenceRC in the
    // coordinate of reverse compelement sequence of the SMRT read.
    subreadSequenceRC.SubreadStart(smrtRead.length - subreadSequence.SubreadEnd());
    subreadSequenceRC.SubreadEnd  (smrtRead.length - subreadSequence.SubreadStart());
    subreadSequenceRC.zmwData     = smrtRead.zmwData;
}

int CountZero(unsigned char *ptr, int length)
{
    int i;
    int nZero = 0;
    for (i = 0; i < length; i++) {
        if (ptr[i] == 0) { ++nZero; }
    }
    return nZero;
}

void MakeVirtualRead(SMRTSequence & smrtRead,
                     const vector<SMRTSequence> & subreads)
{
    assert(subreads.size() > 0);
    DNALength hqStart = 0, hqEnd = 0;
    for(auto subread: subreads) {
        hqStart = min(DNALength(subread.SubreadStart()), hqStart);
        hqEnd   = max(DNALength(subread.SubreadEnd()),   hqEnd);
    }
    smrtRead.Free();
    smrtRead.Allocate(hqEnd);
    smrtRead.lowQualityPrefix = hqStart;
    smrtRead.lowQualitySuffix = smrtRead.length - hqEnd;
    smrtRead.highQualityRegionScore = subreads[0].highQualityRegionScore;
    stringstream ss;
    ss << SMRTTitle(subreads[0].GetTitle()).MovieName() << "/" << subreads[0].HoleNumber();
    smrtRead.CopyTitle(ss.str());
    for (auto subread: subreads) {
        memcpy(&smrtRead.seq[subread.SubreadStart()],
               &subread.seq[0], sizeof(char) * subread.length);
    }
}

void MakeSubreadIntervals(vector<SMRTSequence> & subreads,
                          vector<ReadInterval> & subreadIntervals)
{
    subreadIntervals.clear();
    for (auto subread: subreads) {
        subreadIntervals.push_back(ReadInterval(subread.SubreadStart(),
            subread.SubreadEnd(), subread.highQualityRegionScore));
    }
}

int GetIndexOfMedian(const vector<ReadInterval> & subreadIntervals)
{
    vector<ReadInterval> intervals = subreadIntervals;
    int n = int(subreadIntervals.size() / 2);
    vector<UInt> lens;
    lens.resize(subreadIntervals.size());
    for (auto interval: subreadIntervals)
        lens.push_back(interval.end - interval.start);
    std::sort(lens.begin(), lens.end());
    int pos = 0;
    for (int pos = 0; pos < int(subreadIntervals.size()); pos++)
        if (subreadIntervals[pos].end - subreadIntervals[pos].start == lens[n])
            return pos;
    return pos;
}