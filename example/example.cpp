
#include "AdsLib.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <string.h>
#include <curl/curl.h>

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size * nmemb;
    try
    {
        s->append((char *)contents, newLength);
    }
    catch (std::bad_alloc &e)
    {
        //handle memory problem
        return 0;
    }
    return newLength;
}

std::string getData()
{
    // prints hello world
    CURL *curl;
    CURLcode res;

    /* get a curl handle */
    curl = curl_easy_init();

    if (curl)
    {
        std::string s;
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }

        std::cout << "++++++++++++" << std::endl;
        std::cout << s << std::endl;

        std::cout << "getData finished!" << std::endl;
        /* always cleanup */
        curl_easy_cleanup(curl);
	return s;
    }

    return "";
}

int postData()
{
    // prints hello world
    CURL *curl;
    CURLcode res;
    char *postthis = "badPoint";

    /* get a curl handle */
    curl = curl_easy_init();

    if (curl)
    {
        std::string s;
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);

        res = curl_easy_perform(curl);
        /* Check for errors */
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }

        std::cout << "++++++++++++" << std::endl;
        std::cout << s << std::endl;

        std::cout << "postData finished!" << std::endl;
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return 0;
}


static void NotifyCallback(const AmsAddr* pAddr, const AdsNotificationHeader* pNotification, uint32_t hUser)
{
    const uint8_t* data = reinterpret_cast<const uint8_t*>(pNotification + 1);
    std::cout << std::setfill('0') <<
        "NetId: " << pAddr->netId <<
        " hUser 0x" << std::hex << hUser <<
        " sample time: " << std::dec << pNotification->nTimeStamp <<
        " sample size: " << std::dec << pNotification->cbSampleSize <<
        " value:";
    for (size_t i = 0; i < pNotification->cbSampleSize; ++i) {
        std::cout << " 0x" << std::hex << (int)data[i];
    }
    std::cout << '\n';
}

uint32_t getHandleByNameExample(std::ostream& out, long port, const AmsAddr& server,
                                const std::string handleName)
{
    uint32_t handle = 0;
    const long handleStatus = AdsSyncReadWriteReqEx2(port,
                                                     &server,
                                                     ADSIGRP_SYM_HNDBYNAME,
                                                     0,
                                                     sizeof(handle),
                                                     &handle,
                                                     handleName.size(),
                                                     handleName.c_str(),
                                                     nullptr);
    if (handleStatus) {
        out << "Create handle for '" << handleName << "' failed with: " << std::dec << handleStatus << '\n';
    }
    return handle;
}

void releaseHandleExample(std::ostream& out, long port, const AmsAddr& server, uint32_t handle)
{
    const long releaseHandle = AdsSyncWriteReqEx(port, &server, ADSIGRP_SYM_RELEASEHND, 0, sizeof(handle), &handle);
    if (releaseHandle) {
        out << "Release handle 0x" << std::hex << handle << "' failed with: 0x" << releaseHandle << '\n';
    }
}

uint32_t getSymbolSize(std::ostream& out, long port, const AmsAddr& server, const std::string handleName)
{
    AdsSymbolEntry symbolEntry;
    uint32_t bytesRead;

    const long status = AdsSyncReadWriteReqEx2(port,
                                               &server,
                                               ADSIGRP_SYM_INFOBYNAMEEX,
                                               0,
                                               sizeof(symbolEntry),
                                               &symbolEntry,
                                               handleName.size(),
                                               handleName.c_str(),
                                               &bytesRead);
    if (status) {
        throw std::runtime_error("Unable to determine symbol size, reading ADS symbol information failed with: " + std::to_string(
                                     status));
    }
    return symbolEntry.size;
}

void notificationExample(std::ostream& out, long port, const AmsAddr& server)
{
    const AdsNotificationAttrib attrib = {
        1,
        ADSTRANS_SERVERCYCLE,
        0,
        {4000000}
    };
    uint32_t hNotify;
    uint32_t hUser = 0;

    const long addStatus = AdsSyncAddDeviceNotificationReqEx(port,
                                                             &server,
                                                             0x4020,
                                                             4,
                                                             &attrib,
                                                             &NotifyCallback,
                                                             hUser,
                                                             &hNotify);
    if (addStatus) {
        out << "Add device notification failed with: " << std::dec << addStatus << '\n';
        return;
    }

    std::cout << "Hit ENTER to stop notifications\n";
    std::cin.ignore();

    const long delStatus = AdsSyncDelDeviceNotificationReqEx(port, &server, hNotify);
    if (delStatus) {
        out << "Delete device notification failed with: " << std::dec << delStatus;
        return;
    }
}

void notificationByNameExample(std::ostream& out, long port, const AmsAddr& server)
{
    const AdsNotificationAttrib attrib = {
        1,
        ADSTRANS_SERVERCYCLE,
        0,
        {4000000}
    };
    uint32_t hNotify;
    uint32_t hUser = 0;

    uint32_t handle;

    out << __FUNCTION__ << "():\n";
    handle = getHandleByNameExample(out, port, server, "MAIN.byByte[4]");

    const long addStatus = AdsSyncAddDeviceNotificationReqEx(port,
                                                             &server,
                                                             ADSIGRP_SYM_VALBYHND,
                                                             handle,
                                                             &attrib,
                                                             &NotifyCallback,
                                                             hUser,
                                                             &hNotify);
    if (addStatus) {
        out << "Add device notification failed with: " << std::dec << addStatus << '\n';
        return;
    }

    std::cout << "Hit ENTER to stop by name notifications\n";
    std::cin.ignore();

    const long delStatus = AdsSyncDelDeviceNotificationReqEx(port, &server, hNotify);
    if (delStatus) {
        out << "Delete device notification failed with: " << std::dec << delStatus;
        return;
    }
    releaseHandleExample(out, port, server, handle);
}

void WriteExample(std::ostream& out, long port, const AmsAddr& server,uint16_t wData)
{
    uint16_t buffer = wData;
    std::string data = getData();
    const char* dataChars = data.data();
    char a = 't';
    long size = strlen(dataChars)*sizeof(a);
    out << "size:" << size << '\n';
    out << "size:" << sizeof(a) << '\n';


    const long status = AdsSyncWriteReqEx(port, &server, 0x4020,2,size, dataChars);
    if (status) {
        out << "ADS read failed with: " << wData<< '\n';
        return;
    }
    out << "Write Data: " << wData<< '\n';
}
void readExample(std::ostream& out, long port, const AmsAddr& server, int* armStatusPtr)
{
    uint32_t bytesRead =2;
    uint16_t buffer;

    out << __FUNCTION__ << "():\n";
    const long status = AdsSyncReadReqEx2(port, &server, 0x4020, 0, sizeof(buffer), &buffer, &bytesRead);
    if (*armStatusPtr != status) {
	*armStatusPtr = status;
	switch ((int)status) {
	    case 0:
	    case 4:
	        WriteExample(out, port, server,buffer);
		break;
	    case 3:
		postData();
		break;
	    default:
		break;
	}
    }
    out << "ADS read " << std::dec << bytesRead << " bytes, value: 0x" << std::hex << buffer << '\n';

}



void readByNameExample(std::ostream& out, long port, const AmsAddr& server)
{
    static const char handleName[] = "MAIN.iCount";
    uint32_t bytesRead;

    out << __FUNCTION__ << "():\n";
    const uint32_t handle = getHandleByNameExample(out, port, server, handleName);
    const uint32_t bufferSize = getSymbolSize(out, port, server, handleName);
    const auto buffer = std::unique_ptr<uint8_t>(new uint8_t[bufferSize]);
    for (size_t i = 0; i < 8; ++i) {
        const long status = AdsSyncReadReqEx2(port,
                                              &server,
                                              ADSIGRP_SYM_VALBYHND,
                                              handle,
                                              bufferSize,
                                              buffer.get(),
                                              &bytesRead);
        if (status) {
            out << "ADS read failed with: " << std::dec << status << '\n';
            return;
        }
        out << "ADS read " << std::dec << bytesRead << " bytes:" << std::hex;
        for (size_t i = 0; i < bytesRead; ++i) {
            out << ' ' << (int)buffer.get()[i];
        }
        out << '\n';
    }
    releaseHandleExample(out, port, server, handle);
}

void readStateExample(std::ostream& out, long port, const AmsAddr& server)
{
    uint16_t adsState;
    uint16_t devState;

    const long status = AdsSyncReadStateReqEx(port, &server, &adsState, &devState);
    if (status) {
        out << "ADS read failed with: " << std::dec << status << '\n';
        return;
    }
    out << "ADS state: " << std::dec << adsState << " devState: " << std::dec << devState << '\n';
}

void runExample(std::ostream& out)
{
    static const AmsNetId remoteNetId { 192, 168, 5, 111, 1, 1 };
    static const char remoteIpV4[] = "192.168.5.111";

    // uncomment and adjust if automatic AmsNetId deduction is not working as expected
    //AdsSetLocalAddress({192, 168, 0, 1, 1, 1});

    // add local route to your EtherCAT Master
    if (AdsAddRoute(remoteNetId, remoteIpV4)) {
        out << "Adding ADS route failed, did you specify valid addresses?\n";
        return;
    }

    // open a new ADS port
    const long port = AdsPortOpenEx();
    if (!port) {
        out << "Open ADS port failed\n";
        return;
    }

    const AmsAddr remote { remoteNetId, 801 };
    //notificationExample(out, port, remote);
    //notificationByNameExample(out, port, remote);
    //readExample(out, port, remote);
   // readByNameExample(out, port, remote);
// readStateExample(out, port, remote);
    int armStatus = -1;
    while(true)
    {
       // readStateExample(out, port, remote);
        readExample(out, port, remote, &armStatus);
    }
    const long closeStatus = AdsPortCloseEx(port);
    if (closeStatus) {
        out << "Close ADS port failed with: " << std::dec << closeStatus << '\n';
    }

#ifdef _WIN32
    // WORKAROUND: On Win7 std::thread::join() called in destructors
    //             of static objects might wait forever...
    AdsDelRoute(remoteNetId);
#endif
}

int main()
{
    try {
        runExample(std::cout);
    } catch (const std::runtime_error& ex) {
        std::cout << ex.what() << '\n';
    }
}
