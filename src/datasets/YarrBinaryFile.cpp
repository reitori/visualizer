#include "YarrBinaryFile.h"
#include "AllDataLoaders.h"
#include "logging.h"


namespace
{
    auto logger = logging::make_log("YarrBinaryFile");
    bool YarrBinaryFileRegistered =
      StdDict::registerDataLoader("YarrBinaryFile",
                                []() { return std::unique_ptr<DataLoader>(new YarrBinaryFile());});

}

YarrBinaryFile::YarrBinaryFile() {
    max_events_per_block = (unsigned)(-1);
    block_timeout = 10;
    this_tag = 0;
    this_l1id = 0;
    this_bcid = 0;
    total_events = 0;
    total_hits = 0;
    run_thread = false;
}

YarrBinaryFile::~YarrBinaryFile() {}

void YarrBinaryFile::run() {
    run_thread = true;
    thread_ptr.reset(new std::thread(&YarrBinaryFile::process, this));
}

void YarrBinaryFile::join() {
    run_thread = false;
    thread_ptr->join();
    logger->info("[{}]: Processed {} events, with {} hits, in {} batches", name, total_events, total_hits, batch_n);
}

void YarrBinaryFile::init() {
    this_tag = 0;
    this_l1id = 0;
    this_bcid = 0;
    total_events = 0;
    total_hits = 0;
    run_thread = false;
    header_read = true;
    hit_read = true;
}


void YarrBinaryFile::configure(const json &config) {
    // name
    if(config.contains("name")) {
        name = config["name"]; 
    }
    else {
        logger->error("No name provided! Please add name to all sources. Assuming name = None");
        throw(std::invalid_argument("No name provided!"));
    }

    // block timeout
    if(config.contains("block_timeout"))
        block_timeout = (unsigned)config["block_timeout"];
    else
        block_timeout = 100; // microseconds
    
    // max_events_per_block
    if(config.contains("max_events_per_block"))
        max_events_per_block = (unsigned)config["max_events_per_block"];
    else
        max_events_per_block = -1; 

    // filereader mode
    if(config.contains("read_mode")) {
        std::string mode_str = config["read_mode"];
        if(mode_str == "fast")
            file_rm = read_mode::fast;
        else if(mode_str == "truncated")
            file_rm = read_mode::truncated;
        else {
            logger->error("Unrecognized read mode {}! Options are: [fast, truncated]", mode_str);
            throw(std::invalid_argument("Unrecognized read mode"));
        }
    }
    else
        file_rm = read_mode::fast;

        

    // File stuff
    bool auto_path = false;
    if(config.contains("auto")) {
        auto_path = (bool)config["auto"];
    }
    filename = "";
    if(auto_path)
        filename = (std::string)config["path"] + "/" + name + "_data.raw";
    else
        filename = name;
    fileHandle.open(filename.c_str(), std::istream::in | std::istream::binary);
    filePos = fileHandle.tellg();

    if(!fileHandle.good()) 
        throw(std::invalid_argument("Path " + filename + " could not be opened!"));
}

void YarrBinaryFile::processBatch() {
    int sleep_step = 0;
    bool read_success = true;
    while(fileHandle && run_thread && (curEvents != nullptr) && read_success) { // basic case of "block lives"
        // logger->debug("[{}]: batch variables: fh {} rt {} np {} rs {}", name, (bool)fileHandle, (bool)run_thread, (bool)(curEvents != nullptr), (bool)read_success);
        switch(file_rm) {
            case read_mode::fast:
                fromFile();
                break;
            case read_mode::truncated:
                read_success = fromTruncatedFile();
                break;
            default:
                continue;
        }
        // logger->debug("[{}]: batch after variables: rm {} fh {} rt {} np {} rs {}", name, file_rm, (bool)fileHandle, (bool)run_thread, (bool)(curEvents != nullptr), (bool)read_success);
    }
}

void YarrBinaryFile::process() {
    batch_n = 0;
    
    while(run_thread) {
        // Make new block of events
        if(!curEvents)
            curEvents = std::make_unique<EventData>();
        processBatch();
        if(curEvents->size() > 0) {
            output->pushData(std::move(curEvents));
            batch_n++;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(block_timeout));
    }
}

void YarrBinaryFile::readHeader() {
    fileHandle.read((char*)&this_tag, sizeof(uint32_t));
    fileHandle.read((char*)&this_l1id, sizeof(uint16_t));
    fileHandle.read((char*)&this_bcid, sizeof(uint16_t));
    fileHandle.read((char*)&this_t_hits, sizeof(uint16_t));
}

void YarrBinaryFile::readHits() {
    for (unsigned ii = 0; ii < this_t_hits; ii++) {
        Hit hit;
        fileHandle.read((char*)&hit, sizeof(Hit));
        curEvents->curEvent->addHit(hit);
        total_hits++;
    } // ii
}

void YarrBinaryFile::fromFile() {
    readHeader();
    curEvents->newEvent(this_tag, this_l1id, this_bcid);
    readHits();
    filePos = fileHandle.tellg();
    logger->debug("[{}]: event {}, pos {}: {} | {} | {} | {}", name, total_events, filePos, this_tag, this_l1id, this_bcid, this_t_hits);
    total_events++;
}

bool YarrBinaryFile::fromTruncatedFile() {
    readHeader();

    if(!fileHandle) {
        fileHandle.clear();
        fileHandle.seekg(filePos);
        if(header_read)
            logger->debug("[{}] Failed to read event header - seeking file position {}", name, filePos);
        header_read = false; 
        return false;
    }

    header_read = true;
    
    curEvents->newEvent(this_tag, this_l1id, this_bcid);

    readHits();

    if(!fileHandle) {
        fileHandle.clear();
        fileHandle.seekg(filePos);
        if(hit_read)
            logger->debug("[{}] Failed to read event hits - seeking file position {}", name, filePos);
        return false;
    }
    hit_read = true;

    // logger->debug("[{}]: event {}, pos {}: {} | {} | {} | {}", name, total_events, filePos, this_tag, this_l1id, this_bcid, this_t_hits);
    
    // set file position, increment counters
    filePos = fileHandle.tellg();
    total_events++;
    return true;
}


// #include "EventData.h"

// // std/stl
// #include <iostream>
// #include <fstream>

// void YarrBinaryFile::toFile(std::fstream &handle) const {
//     handle.write((char*)&tag, sizeof(uint32_t));
//     handle.write((char*)&l1id, sizeof(uint16_t));
//     handle.write((char*)&bcid, sizeof(uint16_t));
//     handle.write((char*)&nHits, sizeof(uint16_t));
//     for (auto hit : hits) {
//         handle.write((char*)&hit, sizeof(FrontEndHit));
//     } // h
// }

// void YarrBinaryFile::fromFile(std::fstream &handle) {
//     uint16_t t_hits = 0;
//     handle.read((char*)&tag, sizeof(uint32_t));
//     handle.read((char*)&l1id, sizeof(uint16_t));
//     handle.read((char*)&bcid, sizeof(uint16_t));
//     handle.read((char*)&t_hits, sizeof(uint16_t));
//     for (unsigned ii = 0; ii < t_hits; ii++) {
//         FrontEndHit hit;
//         handle.read((char*)&hit, sizeof(FrontEndHit));
//         this->addHit(hit);
//     } // ii
// }
