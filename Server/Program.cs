using System;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading;

using XBLHServer.Helpers;

namespace XBLHServer
{
    class Program
    {
        private static TcpListener tcpListener;

        private static void Main(string[] args) {
            tcpListener = new TcpListener(IPAddress.Any, 2982);
            new Thread(new ThreadStart(() => CatchClient())).Start();
            Console.WriteLine("API Started & listening");
        }

        private static void CatchClient() {
            tcpListener.Start();
            while (true) {
                Thread.Sleep(100);
                if (tcpListener.Pending()) new Thread(new ThreadStart(() => HandleClient(tcpListener.AcceptTcpClient()))).Start();
            }
        }

        private static string GetConsoleType(byte Type)
        {
            string xboxinfo = "xenon";
            if (Type == 0x10)
                xboxinfo = "zephyr";
            if (Type == 0x20)
                xboxinfo = "falcon";
            if (Type == 0x30)
                xboxinfo = "jasper";
            if (Type == 0x40)
                xboxinfo = "trinity";
            if (Type == 0x50)
                xboxinfo = "corona";

            return xboxinfo;
        }

        private static void HandleClient(TcpClient client) {
            NetworkStream networkStream = client.GetStream();
            SecureStream secureStream = new SecureStream(networkStream);
            string IP = ((IPEndPoint)client.Client.RemoteEndPoint).Address.ToString();

            try
            {
                byte[] Header = new byte[8];
                if (networkStream.Read(Header, 0, 8) != 8) {
                    Tools.AppendText(string.Concat(new object[] { IP, " > ", "BAD Header!!!!", " " }), ConsoleColor.Green);
                    client.Close();
                    return;
                }

                EndianIO IO = new EndianIO(Header, EndianStyle.BigEndian);
                uint Command = IO.Reader.ReadUInt32();
                int Size = IO.Reader.ReadInt32();

                byte[] Data = new byte[Size];
                if ((Size > 0x4800) || (secureStream.Read(Data, 0, Size) != Size)) {
                    Tools.AppendText(string.Concat(new object[] { IP, " > ", "BAD Size!!!!", " " }), ConsoleColor.Green);
                    client.Close();
                    return;
                }

                EndianIO dataStream = new EndianIO(Data, EndianStyle.BigEndian) {
                    Writer = new EndianWriter(secureStream, EndianStyle.BigEndian)
                };


                switch (Command)  
                {
                    case 0x05:
                        {
                            client.Close();
                        }
                        break;
                    case 0x10:
                        {
                            byte[] Version = dataStream.Reader.ReadBytes(4);
                            byte[] CPUKey = dataStream.Reader.ReadBytes(0x10);
                            byte[] SMC = dataStream.Reader.ReadBytes(0x05);
                            byte[] fuseDigest = dataStream.Reader.ReadBytes(0x10);
                            byte[] bootloaderVersion = dataStream.Reader.ReadBytes(0x02);
                            byte[] ModuleDigest = dataStream.Reader.ReadBytes(0x14);

                            byte[] Latest = File.ReadAllBytes("bin/XeX/Client.xex");
                            byte[] Response = new byte[4];

                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "Version: '",              Tools.BytesToHexString(Version), "'" }),            ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "CPUKey: '",               Tools.BytesToHexString(CPUKey), "'" }),             ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "C ModuleDigest: '",       Tools.BytesToHexString(ModuleDigest), "'" }),       ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "S ModuleDigest: '",       Tools.BytesToHexString(Tools.CSHA(Latest)), "'" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "bootloaderVersion: '",    Tools.BytesToHexString(bootloaderVersion), "'" }),  ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "SMC: '",                  Tools.BytesToHexString(SMC), "'" }),                ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "fuseDigest: '",           Tools.BytesToHexString(fuseDigest), "'" }),         ConsoleColor.White);

                            if (ModuleDigest == null)
                            {
                                Buffer.BlockCopy(BitConverter.GetBytes((uint)0x40000000).Reverse().ToArray(), 0, Response, 0, 4);
                                dataStream.Writer.Write(Response);

                                dataStream.Writer.Write(Latest.Length);
                                dataStream.Writer.Write(Latest);
                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "Clients first Boot updating there xex!", " " }), ConsoleColor.Green);
                            }
                            else if (!Tools.CompareBytes(ModuleDigest, Tools.CSHA(Latest)))
                            {
                                Buffer.BlockCopy(BitConverter.GetBytes((uint)0x40000000).Reverse().ToArray(), 0, Response, 0, 4);
                                dataStream.Writer.Write(Response);

                                dataStream.Writer.Write(Latest.Length);
                                dataStream.Writer.Write(Latest);
                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "Clients first Boot updating there xex!", " " }), ConsoleColor.Green);
                            }
                            else
                            {
                                Buffer.BlockCopy(BitConverter.GetBytes((uint)0x4A000000).Reverse().ToArray(), 0, Response, 0, 4);
                                dataStream.Writer.Write(Response);

                                dataStream.Writer.Write(File.ReadAllBytes("bin/patches/patch_xam.bin"));
                                dataStream.Writer.Write(File.ReadAllBytes("bin/patches/patch_cod.bin"));

                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "COMMAND_AUTH", " " }), ConsoleColor.Green);
                            }
                            client.Close(); 
                        }
                        break;
                    case 0x20:
                        {
                            byte[] Title = dataStream.Reader.ReadBytes(4);
                            byte[] CPUKey = dataStream.Reader.ReadBytes(0x10);
                            byte[] ModuleDigest = dataStream.Reader.ReadBytes(0x14);
                            byte[] Gamertag = dataStream.Reader.ReadBytes(0x10);
                            byte[] Latest = File.ReadAllBytes("bin/XeX/Client.xex");

                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "Title: '", Tools.BytesToHexString(Title), "'" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "CPUKey: '", Tools.BytesToHexString(CPUKey), "'" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "ModuleDigest: '", Tools.BytesToHexString(ModuleDigest), "'" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "Gamertag: '", Encoding.UTF8.GetString(Gamertag).Replace("\0", ""), "'" }), ConsoleColor.White);

                            byte[] Response = new byte[0x10];
                            Buffer.BlockCopy(BitConverter.GetBytes((uint)0x4A000000).Reverse().ToArray(), 0, Response, 0, 4);
                            Buffer.BlockCopy(BitConverter.GetBytes((uint)0).Reverse().ToArray(), 0, Response, 4, 4);
                            Buffer.BlockCopy(BitConverter.GetBytes((uint)0).Reverse().ToArray(), 0, Response, 8, 4);
                            Buffer.BlockCopy(BitConverter.GetBytes((uint)0).Reverse().ToArray(), 0, Response, 0xC, 4);
                            dataStream.Writer.Write(Response);

                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "COMMAND_PRES", " " }), ConsoleColor.Green);
                            client.Close();                            
                        }
                        break;
                    case 0x30:
                        {
                            byte[]  SessionToken = dataStream.Reader.ReadBytes(0x14);
                            byte[]  CPUKey = dataStream.Reader.ReadBytes(0x10);
                            byte[]  KVCPUKey = dataStream.Reader.ReadBytes(0x10);
                            byte[]  Salt = dataStream.Reader.ReadBytes(0x10);
                            byte[]  PartNumber = dataStream.Reader.ReadBytes(0x0B);
                            byte[]  KVSignature = dataStream.Reader.ReadBytes(0x100);

                            byte[]  Response = new byte[4];
                            byte[]  responseBuff = new byte[0x100];

                            bool    Crl = Convert.ToBoolean(dataStream.Reader.ReadByte());
                            bool    Fcrt = Convert.ToBoolean(dataStream.Reader.ReadByte());

                            int     hvStatusFlags           = 0x23289d3;
                            int     BldrFlags               = 0xd83e;
                            uint    ConsoleTypeSeqAllow     = 0x304000d;

                            Buffer.BlockCopy(BitConverter.GetBytes((uint)0x4A000000).Reverse().ToArray(), 0, Response, 0, 4);
                            dataStream.Writer.Write(Response);

                            bool Type1KV = true;
                            for (int i = 0x00; i < 0x100; i++) 
                            {
                                if (KVSignature[i] != 0x00) 
                                {
                                    Type1KV = false;
                                }
                            }

                            if (Type1KV == true)
                            {
                                ConsoleTypeSeqAllow = 0x10b0400;
                                BldrFlags = (ushort)(BldrFlags & -33);
                            }

                            hvStatusFlags = (Crl == true) ? (hvStatusFlags | 0x10000) : hvStatusFlags;
                            hvStatusFlags = (Fcrt == true) ? (hvStatusFlags | 0x1000000) : hvStatusFlags;

                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "called 0x30" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "Salt: '", Tools.BytesToHexString(Salt), "'" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "Session Token: '", Tools.BytesToHexString(SessionToken), "'" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "CPUKey: '", Tools.BytesToHexString(CPUKey), "'" }), ConsoleColor.White);

                            byte[] chalresp = File.ReadAllBytes("bin/data/chalresp.bin");
                            byte[] APIChalresp = API.GenerateCleanChallenge(SessionToken, chalresp, Salt, CPUKey, KVCPUKey, Type1KV, Crl, Fcrt);

                            Buffer.BlockCopy(APIChalresp,   0,      responseBuff,  0,      0x100);

                            if (responseBuff[0x28] != 0x4E)
                            {
                                dataStream.Writer.Write(0);
                                client.Close();

                                Tools.AppendText(string.Format("Failed to find responseBuff = NULL\n"), ConsoleColor.Red);
                                Tools.AppendText(string.Concat(new object[] { 0, " > ", "responseBuff: '", Tools.BytesToHexString(responseBuff), "'" }), ConsoleColor.Red);
                            }

                            if (responseBuff[0x29] != 0x4E)
                            {
                                dataStream.Writer.Write(0);
                                client.Close();

                                Tools.AppendText(string.Format("Failed to find responseBuff = NULL\n"), ConsoleColor.Red);
                                Tools.AppendText(string.Concat(new object[] { 0, " > ", "responseBuff: '", Tools.BytesToHexString(responseBuff), "'" }), ConsoleColor.Red);
                            }

                            //Tools.AppendText(string.Concat(new object[] { 0, " > ", "responseBuff: '", Tools.BytesToHexString(responseBuff), "'" }), ConsoleColor.White);
                            dataStream.Writer.Write(responseBuff);

                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "COMMAND_XKEC", " " }), ConsoleColor.Green);
                            client.Close();
                        }
                        break;
                    case 0x40:
                        {
                            int Crl = dataStream.Reader.ReadInt32();
                            int Fcrt = dataStream.Reader.ReadInt32();

                            byte[] Title = dataStream.Reader.ReadBytes(4);
                            byte[] CPUKey = dataStream.Reader.ReadBytes(0x10);
                            byte[] KVDigest = dataStream.Reader.ReadBytes(0x14);

                            byte SerialByte = dataStream.Reader.ReadByte();
                            byte[] ConsoleID = dataStream.Reader.ReadBytes(0x05);
                            byte[] spoofedMacAddress = dataStream.Reader.ReadBytes(0x06);
                            byte[] OddFeatures = dataStream.Reader.ReadBytes(0x02);  // 0x1C, 0x2
                            byte[] bPolicyFlashSize = dataStream.Reader.ReadBytes(0x04);  // 0x24, 0x4
                            byte[] ConsoleSerialNumber = dataStream.Reader.ReadBytes(0x0C); //0xB0, 0xC
                            byte[] GameRegion = dataStream.Reader.ReadBytes(0x2); //0xC8, 0x2
                            byte[] driveIndentifier1 = dataStream.Reader.ReadBytes(0x24); //0xC8A, 0x24
                            byte[] _unk2 = dataStream.Reader.ReadBytes(0x1); //0xC89, 0x1

                            byte[] uCPUKey = dataStream.Reader.ReadBytes(0x10);
                            byte[] orgXOSC = dataStream.Reader.ReadBytes(0x2E0);
                            byte[] cbInp0 = dataStream.Reader.ReadBytes(0x02);
                            byte[] cbInp1 = dataStream.Reader.ReadBytes(0x02);
                            byte[] fuseDigest = dataStream.Reader.ReadBytes(0x10);

                            string xboxinfo = GetConsoleType(orgXOSC[0x1D0]);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "Crl: '", (Crl == 1 ? "TRUE" : "FALSE"), "'", ", ", "Fcrt: '", (Fcrt == 1 ? "TRUE" : "FALSE"), "'" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "Title: '", Tools.BytesToHexString(Title), "'" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "CPUKey: '", Tools.BytesToHexString(CPUKey), "'" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "KVDigest: '", Tools.BytesToHexString(KVDigest), "'" }), ConsoleColor.White);
                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "SerialByte: '", SerialByte, "'" }), ConsoleColor.White);

                            int Console = 0;    //Xenon
                            if (SerialByte <= 0x14 && SerialByte > 0x10)
                                Console = 1;    //Zephyr
                            else if(SerialByte <= 0x18 && SerialByte > 0x14)
                                Console = 2;    //Falcon
                            else if (SerialByte <= 0x52 && SerialByte > 0x18)
                                Console = 3;    //Jasper
                            else if (SerialByte <= 0x58 && SerialByte > 0x52)
                                Console = 4;    //Trinity
                            else if (SerialByte <= 0x70 && SerialByte > 0x58)
                                Console = 5;    //Corona

                            byte[] Response = new byte[4];
                            Buffer.BlockCopy(BitConverter.GetBytes((uint)0x4A000000).Reverse().ToArray(), 0, Response, 0, 4);
                            dataStream.Writer.Write(Response);

                            byte[] XOSC = File.ReadAllBytes("bin/data/xosc.bin");
                            EndianIO xoscStream = new EndianIO(XOSC, EndianStyle.BigEndian);
                            
                            byte[] ExecutionID = null;
                            byte[] TitleHeader = null;

                            if (File.Exists(string.Concat(new object[] { "bin/data/xosc/", Tools.BytesToHexString(Title), ".bin" }))) {
                                ExecutionID = File.ReadAllBytes(string.Concat(new object[] { "bin/data/xosc/", Tools.BytesToHexString(Title), ".bin" })).Take(0x18).ToArray();
                                TitleHeader = File.ReadAllBytes(string.Concat(new object[] { "bin/data/xosc/", Tools.BytesToHexString(Title), ".bin" })).Skip(0x18).Take(File.ReadAllBytes(string.Concat(new object[] { "bin/data/xosc/", Tools.BytesToHexString(Title), ".bin" })).Length - 0x18).ToArray();
                            } else {
                                ExecutionID = File.ReadAllBytes("bin/data/xosc/FFFE07D1.bin").Take(0x18).ToArray();
                                TitleHeader = File.ReadAllBytes("bin/data/xosc/FFFE07D1.bin").Skip(0x18).Take(File.ReadAllBytes("bin/data/xosc/FFFE07D1.bin").Length - 0x18).ToArray();
                            }

                            xoscStream.Writer.Seek(0x38);
                            xoscStream.Writer.Write(ExecutionID);

                            xoscStream.Writer.Seek(0x50);
                            byte[] SMCVersion = { 0x12, 0x62, 0x02, 0x05, 0x00 };
                            if (Console == 0) Buffer.BlockCopy(new byte[] { 0x12, 0x31, 0x01, 0x06, 0x00 }, 0, SMCVersion, 0, 5); //Xenon
                            else if (Console == 1) Buffer.BlockCopy(new byte[] { 0x12, 0x31, 0x01, 0x06, 0x00 }, 0, SMCVersion, 0, 5); //Zephyr
                            else if (Console == 2) Buffer.BlockCopy(new byte[] { 0x12, 0x31, 0x01, 0x06, 0x00 }, 0, SMCVersion, 0, 5); //Falcon
                            else if (Console == 3) Buffer.BlockCopy(new byte[] { 0x12, 0x41, 0x02, 0x03, 0x00 }, 0, SMCVersion, 0, 5); //Jasper
                            else if (Console == 4) Buffer.BlockCopy(new byte[] { 0x12, 0x51, 0x03, 0x01, 0x00 }, 0, SMCVersion, 0, 5);

                            byte[] TitleDigest = API.GenerateCleanTitleDigest(Title, spoofedMacAddress, SerialByte, KVDigest);
                            xoscStream.Writer.Write(TitleDigest);

                            xoscStream.Writer.Seek(0x60);
                            byte[] rawData000x10 = {
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00
                            };
                            xoscStream.Writer.Write(rawData000x10);

                            xoscStream.Writer.Seek(0x70);
                            byte[] FuseDigest = API.GenerateCleanFuseDigest(SerialByte, KVDigest);
                            xoscStream.Writer.Write(FuseDigest);

                            xoscStream.Writer.Seek(0x83);
                            xoscStream.Writer.Write(_unk2);

                            xoscStream.Writer.Seek(0xF0);
                            xoscStream.Writer.Write(driveIndentifier1);

                            xoscStream.Writer.Seek(0x114);
                            xoscStream.Writer.Write(driveIndentifier1);

                            xoscStream.Writer.Seek(0x138);
                            xoscStream.Writer.Write(ConsoleSerialNumber);

                            xoscStream.Writer.Seek(0x146);
                            xoscStream.Writer.Write((ushort)(Fcrt == 1 ? 0xD83E : 0xD81E));

                            xoscStream.Writer.Seek(0x148);
                            xoscStream.Writer.Write(GameRegion);

                            xoscStream.Writer.Seek(0x14A);
                            xoscStream.Writer.Write(OddFeatures);

                            xoscStream.Writer.Seek(0x150);
                            uint PolicyFlashSize = BitConverter.ToUInt32(bPolicyFlashSize, 0);
                            xoscStream.Writer.Write(PolicyFlashSize != 0xFFFFFFFF ? PolicyFlashSize : 0);

                            xoscStream.Writer.Seek(0x158);
                            xoscStream.Writer.Write((uint)(0x23289D3 | (Crl == 1 ? 0x10000 : 0) | (Fcrt == 1 ? 0x1000000 : 0)));

                            ulong PCIEHardwareFlags = 0x4158019002000380;
                            uint HardwareFlags = 0x50000227;
                            if (Console == 0) { PCIEHardwareFlags = 0x2158023102000380; HardwareFlags = 0x00000227; }          //Xenon
                            else if (Console == 1) { PCIEHardwareFlags = 0x2158023102000380; HardwareFlags = 0x10000227; }     //Zephyr
                            else if (Console == 2) { PCIEHardwareFlags = 0x2158023102000380; HardwareFlags = 0x20000227; }     //Falcon
                            else if (Console == 3) { PCIEHardwareFlags = 0x3158116002000380; HardwareFlags = 0x30000227; }     //Jasper
                            else if (Console == 4) { PCIEHardwareFlags = 0x4158016002000380; HardwareFlags = 0x40000227; }     //Trinity

                            xoscStream.Writer.Seek(0x170);
                            xoscStream.Writer.Write(PCIEHardwareFlags);

                            xoscStream.Writer.Seek(0x1A0);
                            xoscStream.Writer.Write(ConsoleID);

                            xoscStream.Writer.Seek(0x1D0);
                            xoscStream.Writer.Write(HardwareFlags);

                            byte[] raw2A8Data = {
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA,
                                0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
                                0x5F, 0x53, 0x47, 0x50, 0xAA, 0xAA, 0xAA, 0xAA
                            };
                            xoscStream.Writer.Seek(0x2A8);
                            xoscStream.Writer.Write(raw2A8Data);

                            dataStream.Writer.Write(TitleDigest);
                            dataStream.Writer.Write(XOSC);

                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "COMMAND_XOSC", " " }), ConsoleColor.Yellow);
                            client.Close();
                        }
                        break;
                    case 0x50:
                        {
                            byte[] CPUKey = dataStream.Reader.ReadBytes(0x10);

                            byte[] Response = new byte[0x0C];
                            byte[] Color = { 0xFF, 0x66, 0x00, 0x66 }; //purple
                            byte[] Background = { 0xFF, 0x39, 0x39, 0x39 };

                            Buffer.BlockCopy(Color, 0, Response, 0x0, 0x04);
                            Buffer.BlockCopy(Background, 0, Response, 0x4, 0x04);
                            Buffer.BlockCopy(BitConverter.GetBytes((uint)0x4A000000).Reverse().ToArray(), 0, Response, 0x8, 0x4);
                            dataStream.Writer.Write(Response);

                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "COMMAND_COLOR", " " }), ConsoleColor.Green);
                            client.Close();
                        }
                        break;
                    case 0x60:
                        {
                            byte[] CPUKey = dataStream.Reader.ReadBytes(0x10);
                            byte[] Title = dataStream.Reader.ReadBytes(0x04);
                            byte[] Response = new byte[4];

                            if (File.Exists("bin/XeX/" + Tools.BytesToHexString(Title) + ".xex")) {
                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "COMMAND_XEX Found File : " + Tools.BytesToHexString(Title), " " }), ConsoleColor.Green);
                                
                                byte[] Latest = File.ReadAllBytes("bin/XeX/" + Tools.BytesToHexString(Title) + ".xex");
                                Buffer.BlockCopy(BitConverter.GetBytes((uint)0x4A000000).Reverse().ToArray(), 0, Response, 0, 4);

                                dataStream.Writer.Write(Response);
                                dataStream.Writer.Write(Latest.Length);
                                dataStream.Writer.Write(Latest);
                            } else {
                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "COMMAND_XEX NOT Found File : " + Tools.BytesToHexString(Title), " " }), ConsoleColor.Green);
                                Buffer.BlockCopy(BitConverter.GetBytes((uint)0x40000000).Reverse().ToArray(), 0, Response, 0, 4);
                                dataStream.Writer.Write(Response);
                            }

                            Tools.AppendText(string.Concat(new object[] { IP, " > ", "COMMAND_XEX", " " }), ConsoleColor.Green);
                            client.Close();
                        }
                        break;
                    case 0x70:
                        {
                            byte[] Version = dataStream.Reader.ReadBytes(4);
                            byte[] CPUKey = dataStream.Reader.ReadBytes(0x10);
                            byte[] ModuleDigest = dataStream.Reader.ReadBytes(0x14);

                            byte[] Latest = File.ReadAllBytes("bin/XeX/XBDM.xex");
                            byte[] Response = new byte[4];

                            if (!Tools.CompareBytes(ModuleDigest, Tools.CSHA(Latest)))
                            {
                                Buffer.BlockCopy(BitConverter.GetBytes((uint)0x40000000).Reverse().ToArray(), 0, Response, 0, 4);
                                dataStream.Writer.Write(Response);

                                dataStream.Writer.Write(Latest.Length);
                                dataStream.Writer.Write(Latest);
                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "Version: '", Tools.BytesToHexString(Version), "'" }), ConsoleColor.White);
                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "CPUKey: '", Tools.BytesToHexString(CPUKey), "'" }), ConsoleColor.White);
                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "ModuleDigest: '", Tools.BytesToHexString(ModuleDigest), "'" }), ConsoleColor.White);
                                Console.Write(string.Format("Clients xbdm does not match protected XBDM updating client XBDM!\n"));

                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "COMMAND_XBDM", " " }), ConsoleColor.Green);
                            }
                            else
                            {
                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "Version: '", Tools.BytesToHexString(Version), "'" }), ConsoleColor.White);
                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "CPUKey: '", Tools.BytesToHexString(CPUKey), "'" }), ConsoleColor.White);
                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "ModuleDigest: '", Tools.BytesToHexString(ModuleDigest), "'" }), ConsoleColor.White);
                                Buffer.BlockCopy(BitConverter.GetBytes((uint)0x4A000000).Reverse().ToArray(), 0, Response, 0, 4);
                                dataStream.Writer.Write(Response);

                                Tools.AppendText(string.Concat(new object[] { IP, " > ", "COMMAND_XBDM", " " }), ConsoleColor.Green);
                            }
                            client.Close();
                        }
                        break;
                }
            }
            catch (Exception ex)
            {
                Tools.AppendText(ex.Message, ConsoleColor.Red);
                if (client.Connected) client.Close();
            }
        }
    }
}
