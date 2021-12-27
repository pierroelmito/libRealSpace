//
//  precomp.h
//  iff
//
//  Created by Fabien Sanglard on 12/27/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef iff_precomp_h
#define iff_precomp_h

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <cfloat>

#include <vector>
#include <stack>
#include <map>
#include <algorithm>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "Math.h"
#include "Matrix.h"
#include "Quaternion.h"

// Reusable Realspace Classes
#include "Base.h"
#include "ByteStream.h"
#include "PakArchive.h"
#include "TreArchive.h"
#include "Camera.h"
#include "Texture.h"
#include "RSImage.h"
#include "IffLexer.h"
#include "RSPalette.h"
#include "RSEntity.h"
#include "RSMusic.h"
#include "RSSound.h"
#include "RSMapTextureSet.h"
#include "RSArea.h"
#include "RLEShape.h"

#include "SCRenderer.h"
extern SCRenderer Renderer;

#endif
