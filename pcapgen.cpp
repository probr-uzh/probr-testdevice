#include <tins/tins.h>
#include <chrono>
#include <thread>
#include <string>
#include <random>
#include <algorithm>

// Version 0.1

using namespace std;
using namespace Tins;

// Some random vendor prefixes (includes Apple, Samsung, Intel)
vector<string> vendors = {"D0:22:BE:", "80:E6:50:", "14:A3:64:", "24:77:03:", "48:E9:F1:"};

// Some random SSIDs
vector<string> ssids = {"Home", "AndroidAP", "Sam's Iphone", "Starbucks", "Hotel Eden WIFI", "FREE WIFI"};

// Holds a list of Mac Adresses used for source/transmitter
vector<string> mac_address_pool;

void print_usage()
{
    cerr << "pcapgen version 0.1\n"
        << "Usage: pcapgen count duration [meantimediff] [stdtimediff]\n"
        << "\tcount\t\tNumber of pcap files to be generated\n"
        << "\t\t\tUse 0 for infinite pcap generation\n"
        << "\tduration\tHow many seconds to record in one file\n"
        << "\tmeantimediff\tMean value for time in milliseconds between two probe requests\n"
        << "\t\t\tLower means more probe requests per second\n"
        << "\t\t\t(Default 500)\n"
        << "\tstdtimediff\tStandard deviation for time in milliseconds between two probe requests\n"
        << "\t\t\t\(Default 1000)\n";
    exit(1);
}

// Gives back a random mac adress in the format XX:XX:XX:XX:XX:XX
string random_mac()
{   
    random_shuffle(vendors.begin(), vendors.end());

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 255);
    stringstream res;
    res << vendors.front();
    for (int n=0; n<3; n++) {
        stringstream ss;
        ss << std::hex
            << std::uppercase
            << std::setw(2)
            << std::setfill('0')
            << dis(gen);
        res << ss.str();
        if (n < 2) res << ":";
    }
    return res.str();
}

int main(int argc, char* argv[])
{
    if (argc < 3)
        print_usage();

    const int NO_OF_PCAP_FILES = atoi(argv[1]);
    const int S_PER_PCAP = atoi(argv[2]);
    int PROBE_MEAN = 500;
    if (argc >= 4)
        PROBE_MEAN = atoi(argv[3]);
    int PROBE_STD = 1000;
    if (argc >= 5)
        PROBE_STD = atoi(argv[4]);

    chrono::seconds pcap_duration(S_PER_PCAP);

    // Add some random MAC adresses to the pool
    for (int i=0; i<20; i++)
        mac_address_pool.push_back(random_mac());

    // Randomization
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    normal_distribution<double> wait_distribution(PROBE_MEAN,PROBE_STD);
    normal_distribution<double> dbm_distribution(-70,20);

    int i = 0;
    while (i<NO_OF_PCAP_FILES || NO_OF_PCAP_FILES == 0) {
        auto start = chrono::system_clock::now();
        chrono::duration<double, milli> elapsed;

        // Get filename for pcap
        stringstream ss;
        ss << "dummy_" 
            << std::chrono::duration_cast<std::chrono::seconds>(start.time_since_epoch()).count()
            << ".pcap";
        string filename = ss.str();

        // Create pcap file
        PacketWriter writer(filename, PacketWriter::RADIOTAP);
        cout << "Creating " << filename << "\n";

        // Insert a packet into pcap in random intervals
        do {
            auto end = chrono::system_clock::now();
            chrono::milliseconds waitTime(std::max(1,(int)round(wait_distribution(generator))));
            this_thread::sleep_for(waitTime);

            Dot11ProbeRequest req = Dot11ProbeRequest();
            req.addr1(Dot11::BROADCAST);                // Destination FF:FF:FF:FF:FF:FF
            random_shuffle(mac_address_pool.begin(), mac_address_pool.end());
            req.addr2(mac_address_pool.front());        // Source (random)
            random_shuffle(ssids.begin(), ssids.end());
            req.ssid(ssids.front());                    // SSID (random)

            RadioTap radio = RadioTap() / req;
            radio.dbm_signal(std::min(-20,(int)round(dbm_distribution(generator))));

            Packet packet = Packet(radio);
            writer.write(packet);


            elapsed = end-start;
        } while(elapsed < pcap_duration);

        i++;
    }
}
