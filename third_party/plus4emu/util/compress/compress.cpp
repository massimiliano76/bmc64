
// compressor utility for Commodore Plus/4 programs
// Copyright (C) 2007-2016 Istvan Varga <istvanv@users.sourceforge.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// The Plus/4 program files generated by this utility are not covered by the
// GNU General Public License, and can be used, modified, and distributed
// without any restrictions.

#include "plus4emu.hpp"
#include "compress.hpp"
#include "compress0.hpp"
#include "compress1.hpp"
#include "compress2.hpp"
#include "compress3.hpp"
#include "compress5.hpp"
#include "decompress0.hpp"
#include "decompress1.hpp"
#include "decompress2.hpp"
#include "decompress3.hpp"
#include "decompress5.hpp"

static void defaultProgressMessageCb(void *userData, const char *msg)
{
  (void) userData;
  if (msg != (char *) 0 && msg[0] != '\0')
    std::fprintf(stderr, "%s\n", msg);
}

static bool defaultProgressPercentageCb(void *userData, int n)
{
  (void) userData;
  if (n != 100)
    std::fprintf(stderr, "\r  %3d%%    ", n);
  else
    std::fprintf(stderr, "\r  %3d%%    \n", n);
  return true;
}

namespace Plus4Compress {

  void Compressor::progressMessage(const char *msg)
  {
    if (msg == (char *) 0)
      msg = "";
    progressMessageCallback(progressMessageUserData, msg);
  }

  bool Compressor::setProgressPercentage(int n)
  {
    n = (n > 0 ? (n < 100 ? n : 100) : 0);
    if (n != prvProgressPercentage) {
      prvProgressPercentage = n;
      return progressPercentageCallback(progressPercentageUserData, n);
    }
    return true;
  }

  Compressor::Compressor(std::vector< unsigned char >& outBuf_)
    : outBuf(outBuf_),
      progressCnt(0),
      progressMax(1),
      progressDisplayEnabled(false),
      prvProgressPercentage(-1),
      progressMessageCallback(&defaultProgressMessageCb),
      progressMessageUserData((void *) 0),
      progressPercentageCallback(&defaultProgressPercentageCb),
      progressPercentageUserData((void *) 0)
  {
    outBuf.resize(0);
  }

  Compressor::~Compressor()
  {
  }

  void Compressor::setCompressionLevel(int n)
  {
    (void) n;
  }

  void Compressor::addZeroPageUpdate(unsigned int endAddr, bool isLastBlock)
  {
    (void) endAddr;
    (void) isLastBlock;
  }

  void Compressor::setProgressMessageCallback(
      void (*func)(void *userData, const char *msg), void *userData_)
  {
    if (func) {
      progressMessageCallback = func;
      progressMessageUserData = userData_;
    }
    else {
      progressMessageCallback = &defaultProgressMessageCb;
      progressMessageUserData = (void *) 0;
    }
  }

  void Compressor::setProgressPercentageCallback(
      bool (*func)(void *userData, int n), void *userData_)
  {
    if (func) {
      progressPercentageCallback = func;
      progressPercentageUserData = userData_;
    }
    else {
      progressPercentageCallback = &defaultProgressPercentageCb;
      progressPercentageUserData = (void *) 0;
    }
  }

  // --------------------------------------------------------------------------

  Decompressor::Decompressor()
  {
  }

  Decompressor::~Decompressor()
  {
  }

  // --------------------------------------------------------------------------

  Compressor * createCompressor(int compressionType,
                                std::vector< unsigned char >& outBuf)
  {
    switch (compressionType) {
    case 0:
      return new Compressor_M0(outBuf);
    case 1:
      return new Compressor_M1(outBuf);
    case 2:
      return new Compressor_M2(outBuf);
    case 3:
      return new Compressor_M3(outBuf);
    case 5:
      return new Compressor_ZLib(outBuf);
    }
    throw Plus4Emu::Exception("internal error: invalid compression type");
  }

  Decompressor * createDecompressor(int compressionType)
  {
    switch (compressionType) {
    case 0:
      return new Decompressor_M0();
    case 1:
      return new Decompressor_M1();
    case 2:
      return new Decompressor_M2();
    case 3:
      return new Decompressor_M3();
    case 5:
      return new Decompressor_ZLib();
    }
    throw Plus4Emu::Exception("internal error: invalid compression type");
  }

  int decompressData(std::vector< std::vector< unsigned char > >& outBuf,
                     const std::vector< unsigned char >& inBuf,
                     int compressionType)
  {
    if (compressionType > 5 || compressionType == 4)
      throw Plus4Emu::Exception("internal error: invalid compression type");
    if (compressionType < 0) {
      // auto-detect compression type
      for (int i = 0; i <= 5; i++) {
        if (i == 3)
          i = 5;
        try {
          decompressData(outBuf, inBuf, i);
          return i;
        }
        catch (Plus4Emu::Exception) {
        }
      }
      compressionType = 3;
    }
    Decompressor  *decomp = createDecompressor(compressionType);
    try {
      decomp->decompressData(outBuf, inBuf);
    }
    catch (...) {
      delete decomp;
      throw;
    }
    delete decomp;
    return compressionType;
  }

  void getSFXModule(std::vector< unsigned char >& outBuf,
                    int compressionType, int runAddr, bool c16Mode,
                    bool noCRCCheck, bool noReadBuffer, int borderEffectType,
                    bool noBlankDisplay, bool noCleanup, bool noROM, bool noCLI)
  {
    switch (compressionType) {
    case 0:
      Decompressor_M0::getSFXModule(outBuf, runAddr, c16Mode,
                                    (noCRCCheck && (noReadBuffer || c16Mode)),
                                    noCleanup, noROM, noCLI);
      break;
    case 1:
      Decompressor_M1::getSFXModule(outBuf, runAddr, c16Mode,
                                    noCRCCheck, noReadBuffer, borderEffectType,
                                    noBlankDisplay, noCleanup, noROM, noCLI);
      break;
    case 2:
      Decompressor_M2::getSFXModule(outBuf, runAddr, c16Mode,
                                    noCRCCheck, noReadBuffer, borderEffectType,
                                    noBlankDisplay, noCleanup, noROM, noCLI);
      break;
    default:
      throw Plus4Emu::Exception("internal error: invalid compression type");
    }
  }

}       // namespace Plus4Compress

