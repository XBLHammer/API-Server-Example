using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using XBLHServer.Helpers;

namespace XBLHServer
{
    public static class API
    {
        public static string apiLink = "http://xbls.us/api/";
        public static string apiKey = "";
        public static int uid = 0;
        public static WebClient Client = new WebClient();

        public static byte[] StringToByteArray(string hex)
        {
            return Enumerable.Range(0, hex.Length)
                             .Where(x => x % 2 == 0)
                             .Select(x => Convert.ToByte(hex.Substring(x, 2), 16))
                             .ToArray();
        }

        public static byte[] GenerateCleanChallenge(byte[] sessionKey, byte[] dirtyChallenge, byte[] ChallengeSalt, byte[] cpuKey, byte[] KVCPUKey, bool Type1KV, bool Crl, bool Fcrt)
        {
            string URL = apiLink + "xke.php?action=getChallengeCertificate&apikey=" + apiKey + "&hvSalt=" + BitConverter.ToString(ChallengeSalt).Replace("-", "") + "&uid=" + uid.ToString() + "&cpuKey=" + BitConverter.ToString(cpuKey).Replace("-", "") + "&sessionsalt=" + BitConverter.ToString(sessionKey).Replace("-", "");
            
            
            string rString = Client.DownloadString(URL);
            byte[] Response = StringToByteArray(rString);

            Buffer.BlockCopy(Response, 0x50, dirtyChallenge, 0x50, 0x14);   //EccHash
            Buffer.BlockCopy(Response, 0x78, dirtyChallenge, 0x78, 0x80);   //RSAData
            Buffer.BlockCopy(Response, 0xF8, dirtyChallenge, 0xF8, 0x02);   //RSAData
            Buffer.BlockCopy(Response, 0xFA, dirtyChallenge, 0xFA, 0x06);   //HVHash

            return dirtyChallenge;
        }

        public static byte[] GenerateCleanChallengeFull(byte[] sessionKey, byte[] ChallengeSalt, byte[] cpuKey, byte[] KVCPUKey, bool Type1KV, bool Crl, bool Fcrt)
        {
            string URL = apiLink + "xke.php?action=getChallengeCertificate&apikey=" + apiKey + "&hvSalt=" + BitConverter.ToString(ChallengeSalt).Replace("-", "") + "&uid=" + uid.ToString() + "&cpuKey=" + BitConverter.ToString(cpuKey).Replace("-", "") + "&sessionsalt=" + BitConverter.ToString(sessionKey).Replace("-", "");

            string rString = Client.DownloadString(URL);
            byte[] Response = StringToByteArray(rString);
            return Response;
        }

        public static byte[] GenerateCleanFuseDigest(byte SerialByte, byte[] KVDigest)
        {
            string URL = apiLink + "xke.php?action=getFuseDigest&SerialByte=" + SerialByte + "&KVDigest=" + BitConverter.ToString(KVDigest).Replace("-", "");

            string rString = Client.DownloadString(URL);
            byte[] Response = StringToByteArray(rString);
            return Response;
        }

        public static byte[] GenerateCleanTitleDigest(byte[] Title, byte[] spoofedMacAddress, byte SerialByte, byte[] KVDigest)
        {
            string URL = apiLink + "xke.php?action=getTitleDigest&Title=" + BitConverter.ToString(Title).Replace("-", "") + "&spoofedMacAddress=" + BitConverter.ToString(spoofedMacAddress).Replace("-", "") + "&SerialByte=" + SerialByte + "&KVDigest=" + BitConverter.ToString(KVDigest).Replace("-", "");

            string rString = Client.DownloadString(URL);
            byte[] Response = StringToByteArray(rString);
            return Response;
        }
    }
}

