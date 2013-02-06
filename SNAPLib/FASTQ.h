/*++

Module Name:

    FASTQ.h

Abstract:

    Headers for fast FASTQ genome "query" reader.

Authors:

    Bill Bolosky, August, 2011

Environment:

    User mode service.

    This class is NOT thread safe.  It's the caller's responsibility to ensure that
    at most one thread uses an instance at any time.

Revision History:

    Adapted from Matei Zaharia's Scala implementation.

--*/

#pragma once

#include "Compat.h"
#include "Read.h"
#include "ReadSupplierQueue.h"
#include "RangeSplitter.h"
#include "DataReader.h"

class   FASTQReader : public ReadReader {
public:

        FASTQReader(DataReader* data, ReadClippingType clipping);

        virtual ~FASTQReader();


        static FASTQReader* create(const DataSupplier* supplier, const char *fileName, _int64 startingOffset, _int64 amountOfFileToProcess,
                                   ReadClippingType clipping = ClipBack);

        bool init(const char* i_fileName);

        static ReadSupplierGenerator *createReadSupplierGenerator(const char *fileName, int numThreads, ReadClippingType clipping = ClipBack, bool gzip = false);
        
        virtual bool getNextRead(Read *readToUpdate);

        virtual void reinit(_int64 startingOffset, _int64 amountOfFileToProcess);
        
        void releaseBefore(DataBatch batch)
        { data->releaseBefore(batch); }

private:
    
        bool skipPartialRecord();

        DataReader*         data;
        const char*         fileName;
        ReadClippingType    clipping;

        static const int maxReadSizeInBytes = 25000;    // Read as in sequencer read, not read-from-the-filesystem.

        static const unsigned maxLineLen = 10000;
        static const unsigned nLinesPerFastqQuery = 4;

        static bool isValidStartingCharacterForNextLine[nLinesPerFastqQuery][256];
        static class _init
        {
        public:
            _init();
        } _initializer;
};

class PairedFASTQReader: public PairedReadReader {
public:
        virtual ~PairedFASTQReader();


        static PairedFASTQReader* create(const DataSupplier* supplier, const char *fileName0, const char *fileName1, _int64 startingOffset, 
                                         _int64 amountOfFileToProcess, ReadClippingType clipping = ClipBack);

        virtual bool getNextReadPair(Read *read0, Read *read1);

        virtual void reinit(_int64 startingOffset, _int64 amountOfFileToProcess) {
            for (int i = 0; i < 2; i++) {
                readers[i]->reinit(startingOffset,amountOfFileToProcess);
            }
        }

        virtual ReadReader *getReaderToInitializeRead(int whichHalfOfPair)
        {
            _ASSERT(0 == whichHalfOfPair || 1 == whichHalfOfPair);
            return readers[whichHalfOfPair];
        }

        static PairedReadSupplierGenerator *createPairedReadSupplierGenerator(const char *fileName0, const char *fileName1, int numThreads, ReadClippingType clipping = ClipBack, bool gzip = false);
 
        void releaseBefore(DataBatch batch)
        { _ASSERT(false); }

private:

        PairedFASTQReader()
        {
            for (int i =0; i < 2; i++) {
                readers[i] = NULL;
            }
        }

        FASTQReader *readers[2];
};

class FASTQWriter { 
public:
        ~FASTQWriter() {fclose(outputFile);};

        static FASTQWriter *Factory(const char *filename);

        bool writeRead(Read *readToWrite);

private:

        FASTQWriter(FILE *i_outputFile) : outputFile(i_outputFile) {}

        FILE *outputFile;
};


