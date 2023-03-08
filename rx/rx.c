#ifndef RX_H
#define RX_H
#define _CRT_SECURE_NO_WARNINGS
#define CINTERFACE
#define COBJMACROS
#define D3D11_NO_HELPERS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
// #define _NO_CRT_STDIO_INLINE
#pragma warning(disable:4115)
#pragma comment(lib, "Ws2_32")
#pragma comment(lib, "user32")
#pragma comment(lib, "Winmm")
#pragma comment(lib, "Kernel32")
#pragma comment(lib, "Advapi32")
#pragma comment(lib, "Xinput")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "d3dcompiler")
#include <windows.h>
#include <Windowsx.h>
#include <hidusage.h>
#include <memoryapi.h>
#include <xinput.h>
#include <timeapi.h>
#include <immintrin.h>
#include <emmintrin.h>
#include <intrin.h>
#include <sysinfoapi.h>
#include <winuser.h>
#include <psapi.h>
#include <ws2tcpip.h>
#include <Winsock2.h>

#include <dxgidebug.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <dxgi1_3.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"


typedef struct rxtexture_t rxtexture_t;
typedef struct rxtexture_t
{ ID3D11Texture2D           * Texture2D;
  ID3D11ShaderResourceView  * Access;
} rxtexture_t;

typedef struct rxvertex_t
{ float x, y;
  unsigned int color;
  float u, v;
} rxvertex_t;

typedef unsigned short rxindex_t;

typedef enum rxdraw_k
{ rxdraw_kCLIP,
  rxdraw_kPRIM,
} rxdraw_k;

typedef struct rxdraw_t rxdraw_t;
typedef struct rxdraw_t
{ rxdraw_k            kind;
  rxtexture_t         texture;
  ID3D11SamplerState *sampler;
  int offset,length;
  int x,y,w,h;
} rxdraw_t;

typedef struct rxuniform_t rxuniform_t;
typedef struct rxuniform_t
{ float e[4][4];
} rxuniform_t;

typedef struct rx_t rx_t;
typedef struct rx_t
{ unsigned                  Quitted:      1;
  unsigned                  Visible:      1;
  unsigned                  Resizable:    1;
  unsigned                  Decorated:    1;
  unsigned                  Floating:     1;
  unsigned                  FocusOnShow:  1;
  unsigned                  EventHolder:  1;
  unsigned                  ClickFocused: 1;
  HWND                      Window;
  void                   *  ControlFiber;
  void                   *  MessageFiber;
  ID3D11InfoQueue        *  InfoQueue;
  ID3D11Device           *  Device;
  ID3D11DeviceContext    *  Context;
  IDXGISwapChain2        *  SwapChain;
  void                   *  FrameAwait;
  ID3D11Texture2D        *  BackBuffer;
  ID3D11RenderTargetView *  RenderTargetView;
  ID3D11RasterizerState  *  RasterizerState;
  ID3D11BlendState       *  BlendState;
  ID3D11VertexShader     *  VertexShader;
  ID3D11PixelShader      *  PixelShader;

  ID3D11InputLayout *VertexShaderInputLayout;

  ID3D11Buffer *VertexBuffer;
  ID3D11Buffer *IndexBuffer;
  ID3D11Buffer *UniformBuffer;

  ID3D11SamplerState *LinearSampler;
  ID3D11SamplerState *PointSampler;

  int         index_buffer[0x1000];
  rxvertex_t vertex_buffer[0x1000];
  rxdraw_t     draw_buffer[0x100];

  int vertex_buffer_index;
  int index_buffer_index;
  int draw_buffer_index;
} rx_t;
static rx_t rx;

void rxdraw(rxtexture_t texture, ID3D11SamplerState *sampler, float x, float y, float w, float h)
{
  rxdraw_t *draw=rx.draw_buffer+rx.draw_buffer_index++;
  draw->kind=rxdraw_kPRIM;
  draw->offset=rx.index_buffer_index;
  draw->length=6;
  draw->texture=texture;
  draw->sampler=sampler;

  int *index=rx.index_buffer+rx.index_buffer_index;
  rx.index_buffer_index+=6;

  index[0]=0;index[3]=0;
  index[1]=1;index[4]=2;
  index[2]=2;index[5]=3;

  rxvertex_t *vert=rx.vertex_buffer+rx.vertex_buffer_index;
  rx.vertex_buffer_index+=4;

  vert[0]=(rxvertex_t){x+0,y+0,0xffffffff,0,1};
  vert[1]=(rxvertex_t){x+0,y+h,0xffffffff,0,0};
  vert[2]=(rxvertex_t){x+w,y+h,0xffffffff,1,0};
  vert[3]=(rxvertex_t){x+w,y+0,0xffffffff,1,1};
}

rxtexture_t rxload_texture(int w, int h, int s, void *m)
{ D3D11_TEXTURE2D_DESC TextureInfo;
  ZeroMemory(&TextureInfo,sizeof(TextureInfo));
  TextureInfo.Width=(unsigned int)w;
  TextureInfo.Height=(unsigned int)h;
  TextureInfo.MipLevels=1;
  TextureInfo.ArraySize=1;
  TextureInfo.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
  TextureInfo.SampleDesc.Count=1;
  TextureInfo.SampleDesc.Quality=0;
  TextureInfo.MiscFlags=0;
  TextureInfo.BindFlags=D3D11_BIND_SHADER_RESOURCE;
  TextureInfo.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;

  D3D11_SUBRESOURCE_DATA SubresourceInfo;
  ZeroMemory(&SubresourceInfo,sizeof(SubresourceInfo));
  SubresourceInfo.pSysMem=m;
  SubresourceInfo.SysMemPitch=s;

  rxtexture_t r;
  ID3D11Device_CreateTexture2D(rx.Device,&TextureInfo,&SubresourceInfo,&r.Texture2D);

  D3D11_SHADER_RESOURCE_VIEW_DESC ViewInfo;
  ZeroMemory(&ViewInfo,sizeof(ViewInfo));
  ViewInfo.Format=DXGI_FORMAT_UNKNOWN;
  ViewInfo.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
  ViewInfo.Texture2D.MostDetailedMip=0;
  ViewInfo.Texture2D.MipLevels=1;

  ID3D11Device_CreateShaderResourceView(rx.Device,(ID3D11Resource *)r.Texture2D,&ViewInfo,&r.Access);
  return r;
}

rxtexture_t rxload_texture_file(const char *name)
{
  int x,y,n;
  unsigned char *m;
  rxtexture_t r;

  m=stbi_load(name,&x,&y,&n,4);
  r=rxload_texture(x,y,x*4,m);
  stbi_image_free(m);
  return r;
}

void rxtick()
{
  MSG Message;
  while(PeekMessage(&Message,NULL,0,0,PM_REMOVE))
  { TranslateMessage(&Message);
    DispatchMessageW(&Message);
  }

  RECT WindowClient;
  GetClientRect(rx.Window,&WindowClient);

  UINT WindowWidth,WindowHeight;
  WindowWidth=WindowClient.right-WindowClient.left;
  WindowHeight=WindowClient.bottom-WindowClient.top;

  float Color[4]={1.f,0.f,0.f,1.f};
  ID3D11DeviceContext_ClearRenderTargetView(rx.Context,rx.RenderTargetView,Color);

  if(!rx.UniformBuffer)
  {
    rxuniform_t uniform;
    uniform.e[0][0]=2.0f/(WindowWidth);
    uniform.e[0][1]=0.f;
    uniform.e[0][2]=0.f;
    uniform.e[0][3]=0.f;

    uniform.e[1][0]=0.f;
    uniform.e[1][1]=2.0f/(WindowHeight);
    uniform.e[1][2]=0.f;
    uniform.e[1][3]=0.f;

    uniform.e[2][0]=0.f;
    uniform.e[2][1]=0.f;
    uniform.e[2][2]=.5f;
    uniform.e[2][3]=0.f;

    uniform.e[3][0]=-1.f;
    uniform.e[3][1]=-1.f;
    uniform.e[3][2]=.0f;
    uniform.e[3][3]=1.f;


    D3D11_BUFFER_DESC BufferInfo;
    BufferInfo.Usage=D3D11_USAGE_DEFAULT;
    BufferInfo.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
    BufferInfo.CPUAccessFlags=0;
    BufferInfo.MiscFlags=0;
    BufferInfo.ByteWidth=sizeof(uniform);
    BufferInfo.StructureByteStride=0;

    D3D11_SUBRESOURCE_DATA SubresourceInfo;
    SubresourceInfo.pSysMem=&uniform;
    SubresourceInfo.SysMemPitch=0;
    SubresourceInfo.SysMemSlicePitch=0;
    ID3D11Device_CreateBuffer(rx.Device,&BufferInfo,&SubresourceInfo,&rx.UniformBuffer);
  }

  if(!rx.IndexBuffer)
  { D3D11_BUFFER_DESC BufferInfo;
    BufferInfo.Usage=D3D11_USAGE_DYNAMIC;
    BufferInfo.BindFlags=D3D11_BIND_INDEX_BUFFER;
    BufferInfo.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
    BufferInfo.MiscFlags=0;
    BufferInfo.ByteWidth=sizeof(rx.index_buffer);
    BufferInfo.StructureByteStride=0;

    D3D11_SUBRESOURCE_DATA SubresourceInfo;
    SubresourceInfo.pSysMem=rx.index_buffer;
    SubresourceInfo.SysMemPitch=0;
    SubresourceInfo.SysMemSlicePitch=0;
    ID3D11Device_CreateBuffer(rx.Device,&BufferInfo,&SubresourceInfo,&rx.IndexBuffer);
  }

  if(!rx.VertexBuffer)
  { D3D11_BUFFER_DESC BufferInfo;
    BufferInfo.Usage=D3D11_USAGE_DYNAMIC;
    BufferInfo.BindFlags=D3D11_BIND_VERTEX_BUFFER;
    BufferInfo.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
    BufferInfo.MiscFlags=0;
    BufferInfo.ByteWidth=sizeof(rx.vertex_buffer);
    BufferInfo.StructureByteStride=0;

    D3D11_SUBRESOURCE_DATA SubresourceInfo;
    SubresourceInfo.pSysMem=rx.vertex_buffer;
    SubresourceInfo.SysMemPitch=0;
    SubresourceInfo.SysMemSlicePitch=0;
    ID3D11Device_CreateBuffer(rx.Device,&BufferInfo,&SubresourceInfo,&rx.VertexBuffer);
  }

  D3D11_MAPPED_SUBRESOURCE VertexMapped,IndexMapped;
  ID3D11DeviceContext_Map(rx.Context,(ID3D11Resource*)rx.VertexBuffer,0,D3D11_MAP_WRITE_DISCARD,0,&VertexMapped);
  ID3D11DeviceContext_Map(rx.Context,(ID3D11Resource*)rx.IndexBuffer,0,D3D11_MAP_WRITE_DISCARD,0,&IndexMapped);
  memcpy(VertexMapped.pData,rx.vertex_buffer,sizeof(rx.vertex_buffer));
  memcpy(IndexMapped.pData,rx.index_buffer,sizeof(rx.index_buffer));
  ID3D11DeviceContext_Unmap(rx.Context,(ID3D11Resource*)rx.VertexBuffer,0);
  ID3D11DeviceContext_Unmap(rx.Context,(ID3D11Resource*)rx.IndexBuffer,0);


  D3D11_VIEWPORT Viewport;
  ZeroMemory(&Viewport,sizeof(Viewport));
  Viewport.Width=(float)WindowWidth;
  Viewport.Height=(float)WindowHeight;
  Viewport.MinDepth=0;
  Viewport.MaxDepth=1;

  ID3D11DeviceContext_RSSetState(rx.Context,rx.RasterizerState);
  ID3D11DeviceContext_RSSetViewports(rx.Context,1,&Viewport);
  ID3D11DeviceContext_OMSetRenderTargets(rx.Context,1,&rx.RenderTargetView,0);

  unsigned int Stride=sizeof(rxvertex_t);
  unsigned int Offset=0;
  ID3D11DeviceContext_IASetInputLayout(rx.Context,rx.VertexShaderInputLayout);
  ID3D11DeviceContext_IASetVertexBuffers(rx.Context,0,1,&rx.VertexBuffer,&Stride,&Offset);
  ID3D11DeviceContext_IASetIndexBuffer(rx.Context,rx.IndexBuffer,DXGI_FORMAT_R32_UINT,0);
  ID3D11DeviceContext_IASetPrimitiveTopology(rx.Context,D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  ID3D11DeviceContext_VSSetConstantBuffers(rx.Context,0,1,&rx.UniformBuffer);
  ID3D11DeviceContext_PSSetConstantBuffers(rx.Context,0,1,&rx.UniformBuffer);
  ID3D11DeviceContext_VSSetShader(rx.Context,rx.VertexShader,0x00,0);
  ID3D11DeviceContext_PSSetShader(rx.Context,rx.PixelShader,0x00,0);

  D3D11_RECT RootClip;
  RootClip.left=0;
  RootClip.top=0;
  RootClip.right=0xffffff;
  RootClip.bottom=0xffffff;
  ID3D11DeviceContext_RSSetScissorRects(rx.Context,1,&RootClip);

  rxdraw_t *draw;
  for (draw=rx.draw_buffer;draw<rx.draw_buffer+rx.draw_buffer_index;++draw)
  { if(draw->kind==rxdraw_kCLIP)
    {
      D3D11_RECT R;//  = { (u32) ClipMin.X, (u32) ClipMin.Y, (u32) ClipMax.X, (u32) ClipMax.Y };
      ID3D11DeviceContext_RSSetScissorRects(rx.Context,1,&R);
    } else
    if(draw->kind==rxdraw_kPRIM)
    { ID3D11DeviceContext_PSSetSamplers(rx.Context,0,1,&draw->sampler);
      ID3D11DeviceContext_PSSetShaderResources(rx.Context,0,1,&draw->texture.Access);
      ID3D11DeviceContext_DrawIndexed(rx.Context,draw->length,draw->offset,0);
    }
  }

  IDXGISwapChain_Present(rx.SwapChain,1u,0);
  WaitForSingleObjectEx(rx.FrameAwait,33,TRUE);

  ShowWindow(rx.Window,SW_SHOW);

  rx.vertex_buffer_index=0;
  rx.index_buffer_index=0;
  rx.draw_buffer_index=0;
}

LRESULT CALLBACK
rxwindow_callback_win32(HWND NativeWindow,UINT Message,WPARAM wParam,LPARAM lParam)
{ // rxwindow_t *window=(rxwindow_t*)GetWindowLongPtrA(NativeWindow,GWLP_USERDATA);
  return DefWindowProcW(NativeWindow,Message,wParam,lParam);
}

void rxinit(const wchar_t *WindowTitle)
{ UINT DriverModeFlags=
    D3D11_CREATE_DEVICE_DEBUG| // -- Note: COMMENT THIS OUT TO USE INTEL'S GRAPHIC ANALYZER
    D3D11_CREATE_DEVICE_SINGLETHREADED|D3D11_CREATE_DEVICE_BGRA_SUPPORT;
  D3D_FEATURE_LEVEL DriverFeatureMenu[2][2]=
  { {D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0},
    {D3D_FEATURE_LEVEL_10_1,D3D_FEATURE_LEVEL_10_0},
  };
  D3D_FEATURE_LEVEL DriverSelectedFeatureLevel;
  if(SUCCEEDED(D3D11CreateDevice(NULL,D3D_DRIVER_TYPE_HARDWARE,0,DriverModeFlags,DriverFeatureMenu[0],
      ARRAYSIZE(DriverFeatureMenu[0]),D3D11_SDK_VERSION,&rx.Device,&DriverSelectedFeatureLevel,&rx.Context))||
     SUCCEEDED(D3D11CreateDevice(NULL,D3D_DRIVER_TYPE_WARP,0,DriverModeFlags,DriverFeatureMenu[1],
      ARRAYSIZE(DriverFeatureMenu[1]),D3D11_SDK_VERSION,&rx.Device,&DriverSelectedFeatureLevel,&rx.Context)))
  { if((DriverModeFlags&D3D11_CREATE_DEVICE_DEBUG))
    { if(SUCCEEDED(IProvideClassInfo_QueryInterface(rx.Device,&IID_ID3D11InfoQueue,(void**)&rx.InfoQueue)))
      { ID3D11InfoQueue_SetBreakOnSeverity(rx.InfoQueue, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        ID3D11InfoQueue_SetBreakOnSeverity(rx.InfoQueue, D3D11_MESSAGE_SEVERITY_ERROR,      TRUE);
        ID3D11InfoQueue_SetBreakOnSeverity(rx.InfoQueue, D3D11_MESSAGE_SEVERITY_WARNING,    TRUE);
      }
    }
  }

  WNDCLASSW WindowClass;
  ZeroMemory(&WindowClass,sizeof(WindowClass));
  WindowClass.lpfnWndProc=rxwindow_callback_win32;
  WindowClass.hInstance=GetModuleHandleW(NULL);
  WindowClass.lpszClassName=WindowTitle;
  RegisterClassW(&WindowClass);

  rx.Window=CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP,WindowClass.lpszClassName,WindowTitle,
    WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
      NULL,NULL,WindowClass.hInstance,NULL);

  SetLastError(S_OK);
  SetWindowLongPtrA(rx.Window,GWLP_USERDATA,(LONG_PTR)&rx);
  SetLastError(S_OK);
  GetWindowLongPtrA(rx.Window,GWLP_USERDATA);

  RECT WindowClient;
  UINT WindowWidth,WindowHeight;
  GetClientRect(rx.Window,&WindowClient);
  WindowWidth=WindowClient.right-WindowClient.left;
  WindowHeight=WindowClient.bottom-WindowClient.top;

  IDXGIFactory2 *DXGIFactory=NULL;
  IDXGIDevice *DXGIDevice=NULL;
  IDXGIAdapter *DXGIAdapter=NULL;
  ID3D11Device_QueryInterface(rx.Device,&IID_IDXGIDevice,(void **)&DXGIDevice);
  IDXGIDevice_GetAdapter(DXGIDevice,&DXGIAdapter);
  IDXGIAdapter_GetParent(DXGIAdapter,&IID_IDXGIFactory2,(void**)&DXGIFactory);
  IDXGIAdapter_Release(DXGIAdapter);
  IDXGIDevice_Release(DXGIDevice);

  DXGI_SWAP_CHAIN_DESC1 SwapchainInfo;
  ZeroMemory(&SwapchainInfo,sizeof(SwapchainInfo));
  SwapchainInfo.Width=WindowWidth;
  SwapchainInfo.Height=WindowHeight;
  SwapchainInfo.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
  SwapchainInfo.SampleDesc.Count=1;
  SwapchainInfo.SampleDesc.Quality=0;
  SwapchainInfo.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT; // DXGI_USAGE_UNORDERED_ACCESS
  SwapchainInfo.BufferCount=2;
  SwapchainInfo.SwapEffect= DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  SwapchainInfo.Flags=DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH|DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

  DXGI_SWAP_CHAIN_FULLSCREEN_DESC SwapchainFullscreenInfo;
  ZeroMemory(&SwapchainFullscreenInfo,sizeof(SwapchainFullscreenInfo));
  SwapchainFullscreenInfo.RefreshRate.Numerator=1;
  SwapchainFullscreenInfo.RefreshRate.Denominator=1;
  SwapchainFullscreenInfo.Windowed=TRUE;

  IDXGIFactory2_CreateSwapChainForHwnd(DXGIFactory,(IUnknown *)rx.Device,rx.Window,
    &SwapchainInfo,&SwapchainFullscreenInfo,NULL,(IDXGISwapChain1 **)&rx.SwapChain);

  rx.FrameAwait=IDXGISwapChain2_GetFrameLatencyWaitableObject(rx.SwapChain);

  IDXGISwapChain_GetBuffer(rx.SwapChain,0,&IID_ID3D11Texture2D,(void **)&rx.BackBuffer);
  IDXGIFactory_Release(DXGIFactory);

  D3D11_RENDER_TARGET_VIEW_DESC RenderTargetViewInfo;
  ZeroMemory(&RenderTargetViewInfo,sizeof(RenderTargetViewInfo));
  RenderTargetViewInfo.Format=DXGI_FORMAT_UNKNOWN;
  RenderTargetViewInfo.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
  ID3D11Device_CreateRenderTargetView(rx.Device,(ID3D11Resource*)rx.BackBuffer,&RenderTargetViewInfo,&rx.RenderTargetView);

  D3D11_RASTERIZER_DESC RasterizerInfo;
  ZeroMemory(&RasterizerInfo,sizeof(RasterizerInfo));
  RasterizerInfo.FillMode=D3D11_FILL_SOLID;
  RasterizerInfo.CullMode=D3D11_CULL_NONE;
  RasterizerInfo.FrontCounterClockwise=0;
  RasterizerInfo.DepthBias=0;
  RasterizerInfo.DepthBiasClamp=0;
  RasterizerInfo.SlopeScaledDepthBias=0;
  RasterizerInfo.DepthClipEnable=TRUE;
  RasterizerInfo.ScissorEnable=TRUE;
  RasterizerInfo.MultisampleEnable=FALSE;
  RasterizerInfo.AntialiasedLineEnable=FALSE;
  ID3D11Device_CreateRasterizerState(rx.Device,&RasterizerInfo,&rx.RasterizerState);

  D3D11_RENDER_TARGET_BLEND_DESC TargetBlendStateInfo;
  ZeroMemory(&TargetBlendStateInfo,sizeof(TargetBlendStateInfo));
  TargetBlendStateInfo.BlendEnable=TRUE;
  TargetBlendStateInfo.SrcBlend=D3D11_BLEND_SRC_ALPHA;
  TargetBlendStateInfo.DestBlend=D3D11_BLEND_INV_SRC_ALPHA;
  TargetBlendStateInfo.BlendOp=D3D11_BLEND_OP_ADD;
  TargetBlendStateInfo.SrcBlendAlpha=D3D11_BLEND_ZERO;
  TargetBlendStateInfo.DestBlendAlpha=D3D11_BLEND_ZERO;
  TargetBlendStateInfo.BlendOpAlpha=D3D11_BLEND_OP_ADD;
  TargetBlendStateInfo.RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;

  D3D11_BLEND_DESC BlendStateInfo;
  ZeroMemory(&BlendStateInfo,sizeof(BlendStateInfo));
  BlendStateInfo.RenderTarget[0]=TargetBlendStateInfo;
  ID3D11Device_CreateBlendState(rx.Device,&BlendStateInfo,&rx.BlendState);

  const char *ShaderCode=
  "cbuffer ConstBuffer : register(b0)"                                      "\r\n"
  "{ float4x4 ProjectionMatrix;"                                            "\r\n"
  "};"                                                                      "\r\n"
  "struct VS_INPUT"                                                         "\r\n"
  "{ float2 pos : POSITION;"                                                "\r\n"
  "  float4 col : COLOR0;"                                                  "\r\n"
  "  float2 uv  : TEXCOORD0;"                                               "\r\n"
  "};"                                                                      "\r\n"
  "struct PS_INPUT"                                                         "\r\n"
  "{ float4 pos : SV_POSITION;"                                             "\r\n"
  "  float4 col : COLOR0;"                                                  "\r\n"
  "  float2 uv  : TEXCOORD0;"                                               "\r\n"
  "};"                                                                      "\r\n"
  "PS_INPUT MainVS(VS_INPUT input)"                                         "\r\n"
  "{ PS_INPUT output;"                                                      "\r\n"
  "  output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));"  "\r\n"
  "  output.col = input.col;"                                               "\r\n"
  "  output.uv  = input.uv;"                                                "\r\n"
  "  return output;"                                                        "\r\n"
  "}"                                                                       "\r\n"
  "SamplerState      sampler0;"                                             "\r\n"
  "Texture2D<float4> texture0;"                                             "\r\n"
  "float4 MainPS(PS_INPUT input) : SV_Target"                               "\r\n"
  "{ float4 out_col = texture0.Sample(sampler0, input.uv);"                 "\r\n"
  "  return out_col;"                                                       "\r\n"
  "}"                                                                       "\r\n";

  UINT CompilationFlags=D3DCOMPILE_DEBUG|D3DCOMPILE_SKIP_OPTIMIZATION;
  ID3DBlob *ShaderBlob,*MessageBlob;

  if(SUCCEEDED(D3DCompile(ShaderCode,strlen(ShaderCode),"rx::vertex-shader",0,0,"MainVS","vs_5_0",
    CompilationFlags,0,&ShaderBlob,&MessageBlob)))
  { ID3D11Device_CreateVertexShader(rx.Device,ID3D10Blob_GetBufferPointer(ShaderBlob),
      ID3D10Blob_GetBufferSize(ShaderBlob),NULL,&rx.VertexShader);
  } else
  { OutputDebugStringA((char*)ID3D10Blob_GetBufferPointer(MessageBlob));
    OutputDebugStringA("\r\n");
  }

  D3D11_INPUT_ELEMENT_DESC LayoutElements[]=
  { (D3D11_INPUT_ELEMENT_DESC){"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
    (D3D11_INPUT_ELEMENT_DESC){"COLOR",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
    (D3D11_INPUT_ELEMENT_DESC){"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
  };

  ID3D11Device_CreateInputLayout(rx.Device,LayoutElements,ARRAYSIZE(LayoutElements),
    ID3D10Blob_GetBufferPointer(ShaderBlob),ID3D10Blob_GetBufferSize(ShaderBlob),&rx.VertexShaderInputLayout);

  if(SUCCEEDED(D3DCompile(ShaderCode,strlen(ShaderCode),"rx::pixel-shader",0,0,"MainPS","ps_5_0",
    CompilationFlags,0,&ShaderBlob,&MessageBlob)))
  { ID3D11Device_CreatePixelShader(rx.Device,ID3D10Blob_GetBufferPointer(ShaderBlob),
      ID3D10Blob_GetBufferSize(ShaderBlob),NULL,&rx.PixelShader);
  } else
  { OutputDebugStringA((char*)ID3D10Blob_GetBufferPointer(MessageBlob));
    OutputDebugStringA("\r\n");
  }



  D3D11_SAMPLER_DESC SamplerInfo;
  ZeroMemory(&SamplerInfo,sizeof(SamplerInfo));
  SamplerInfo.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;
  SamplerInfo.AddressV=D3D11_TEXTURE_ADDRESS_WRAP;
  SamplerInfo.AddressW=D3D11_TEXTURE_ADDRESS_WRAP;

  SamplerInfo.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  ID3D11Device_CreateSamplerState(rx.Device,&SamplerInfo,&rx.LinearSampler);

  SamplerInfo.Filter=D3D11_FILTER_MIN_MAG_MIP_POINT;
  ID3D11Device_CreateSamplerState(rx.Device,&SamplerInfo,&rx.PointSampler);
}

#endif











// #ifndef _ZEN_WIN
// #define _ZEN_WIN

// #ifndef ZEN_DEFAULT_WINDOW_STYLE_WIN32
// # define ZEN_DEFAULT_WINDOW_STYLE_WIN32 WS_OVERLAPPEDWINDOW
// #endif

// ZEN_FUNCTION void
// ZenExitNativeProcess(int Code)
// { ExitProcess(Code);
// }

// // TODO(RJ): REMOVE THIS FROM HERE!
// ZEN_FUNCTION void *
// ZenGetSystemStandardOutputHandle()
// { return (void *) GetStdHandle(STD_OUTPUT_HANDLE);
// }

// ZEN_FUNCTION void *
// ZenGetSystemStandardInputHandle()
// { return (void *) GetStdHandle(STD_INPUT_HANDLE);
// }

// ZEN_FUNCTION void *
// ZenGetSystemStandardErrorHandle()
// { return (void *) GetStdHandle(STD_ERROR_HANDLE);
// }

// ZEN_FUNCTION void *
// ZenCreateNativeEvent(int AutoReset,int IsSignaled,ConstStrA Name)
// { return (void *) CreateEventA(NULL,!AutoReset,IsSignaled,Name);
// }

// static int
// ZenResetNativeEvent(void *Event)
// { return ResetEvent((HANDLE)Event);
// }

// ZEN_FUNCTION int
// ZenConsumeNativeEvent(void *Event)
// { DWORD Code;
//   Code=WaitForSingleObject((HANDLE)Event,INFINITE);
//   return (Code==WAIT_OBJECT_0);
// }

// static int
// ZenProduceNativeEvent(void *Event)
// { return SetEvent((HANDLE)Event);
// }

// ZEN_FUNCTION void *
// ZenCreateNativeMutex(int InitialOwner, ConstStrA Name)
// { return (void *) CreateMutexA(NULL,InitialOwner,Name);
// }

// ZEN_FUNCTION int
// ZenAcquireNativeMutex(void *Mutex)
// { DWORD Code;
//   for(Code=WAIT_TIMEOUT;Code==WAIT_TIMEOUT;
//       Code=WaitForSingleObject((HANDLE)Mutex,INFINITE));
//   return (Code==WAIT_OBJECT_0);
// }

// static int
// ZenReleaseNativeMutex(void *Mutex)
// { return ReleaseMutex((HANDLE)Mutex);
// }

// static void
// ZenReleaseNativeMutexEnsured(void *Mutex)
// { int Released;
//   Released=ZenReleaseNativeMutex(Mutex);
//   AssertW(Released,L"unreleased mutex");
//   (void)Released;
// }


// // TODO(RJ): REMOVE THIS FROM HERE!
// ZEN_FUNCTION u64
// ZenGetNativePerformanceCounter()
// { LARGE_INTEGER I;
//   QueryPerformanceCounter(& I);
//   return I.QuadPart;
// }

// ZEN_FUNCTION u64
// ZenGetNativePerformanceFrequency()
// { LARGE_INTEGER I;
//   QueryPerformanceFrequency(& I);
//   return I.QuadPart;
// }

// ZEN_FUNCTION VOID
// ZenDebugInternalFiberFun_win32(LPVOID lpCore)
// { ZenCore *Core = (ZenCore *) lpCore;
//   for(;;)
//   { if(Core->Scheduler.DebugInternalRoutine)
//     { Core->Scheduler.DebugInternalResult = Core->Scheduler.DebugInternalRoutine(Core, Core->Scheduler.DebugInternalUser);
//     }
//   }
// }

// ZEN_FUNCTION VOID
// ZenDebugExternalFiberFun_win32(LPVOID lpCore)
// { ZenCore *Core = (ZenCore *) lpCore;
//   for(;;)
//   { if(Core->Scheduler.DebugExternalRoutine)
//     { Core->Scheduler.DebugExternalResult = Core->Scheduler.DebugExternalRoutine(Core, Core->Scheduler.DebugExternalUser);
//     }
//   }
// }

// ZEN_FUNCTION int
// ZenExternalCoreInitialize(ZenExternalCore *Core)
// {
//   // Note: normally, you'd have the platform side initialize this, but since we have a platform api ...
//   Core->Scheduler.ControlFiber       = ConvertThreadToFiber(Core);
//   Core->Scheduler.DebugInternalFiber = CreateFiber(0, ZenDebugInternalFiberFun_win32,Core);
//   Core->Scheduler.DebugExternalFiber = CreateFiber(0, ZenDebugExternalFiberFun_win32,Core);

//   // Todo: see how glw3 does it
//   typedef BOOL WINAPI _YYY_(void);
//   typedef BOOL WINAPI _XXX_(DPI_AWARENESS_CONTEXT);
//   HMODULE User32 = LoadLibraryA("user32.dll");
//   if(_XXX_ *XXX = (_XXX_ *) GetProcAddress(User32, "SetProcessDPIAwarenessContext"))
//   { XXX(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
//   } else
//   if(_YYY_ *YYY = (_YYY_ *) GetProcAddress(User32, "SetProcessDPIAware"))
//   { YYY();
//   }
//   FreeLibrary(User32);
//   return 1;
// }


// // Todo:?
// #if defined(ZEN_APPMODE_CONSOLE)
// int
// main(int argc, char **argv)
// { ZenMain(argc,argv);
//   return 0;
// }
// #elif defined(ZEN_APPMODE_WINDOWED)
// INT32 WINAPI
// WinMain(HINSTANCE inst, HINSTANCE prev_inst, PSTR cmdl, INT cmds)
// { char  lbuf[0x100];
//   char *sbuf[0x020];
//   char *ccur,*lcur,**scur;
//   ccur=cmdl;
//   lcur=lbuf;
//   scur=sbuf;
//   while(*ccur)
//   { *scur++=lcur;
//     while(*ccur && *ccur != ' ') *lcur++ = *ccur++;
//     *lcur++ = 0;
//     while(*ccur && *ccur == ' ') ccur++;
//   }

//   ZenMain((i32)(lcur-lbuf),sbuf);
//   return 0;
// }
// #elif defined(ZEN_APPMODE_DYNAMIC)
// static INT32 WINAPI
// DllMain(HINSTANCE Inst, DWORD Reason, LPVOID Reserved)
// { switch( Reason )
//   { case DLL_PROCESS_ATTACH:
//     break;
//     case DLL_THREAD_ATTACH:
//     break;
//     case DLL_THREAD_DETACH:
//     break;
//     case DLL_PROCESS_DETACH:
//     break;
//   }
//   ZenMain(0,0);
//   return 0;
// }
// #endif

// static i32 ZenInternalizeWindowEvent_win32(DWORD Message, WPARAM wParam)
// { switch(Message)
//   { case WM_LBUTTONUP: case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK: return ZenEvent_MouseButton_0;
//     case WM_RBUTTONUP: case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK: return ZenEvent_MouseButton_1;
//     case WM_MBUTTONUP: case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK: return ZenEvent_MouseButton_2;
//     case WM_XBUTTONUP: case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
//       return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? ZenEvent_MouseButton_3 : ZenEvent_MouseButton_4;
//     case WM_KEYUP: case WM_KEYDOWN:
//     { switch (wParam)
//       { default: return ZenEvent_None;
//         case '0': return ZenEvent_0; case '1': return ZenEvent_1; case '2': return ZenEvent_2; case '3': return ZenEvent_3; case '4': return ZenEvent_4;
//         case '5': return ZenEvent_5; case '6': return ZenEvent_6; case '7': return ZenEvent_7; case '8': return ZenEvent_8; case '9': return ZenEvent_9;
//         case 'A': return ZenEvent_A; case 'B': return ZenEvent_B; case 'C': return ZenEvent_C; case 'D': return ZenEvent_D; case 'E': return ZenEvent_E;
//         case 'F': return ZenEvent_F; case 'G': return ZenEvent_G; case 'H': return ZenEvent_H; case 'I': return ZenEvent_I; case 'J': return ZenEvent_J;
//         case 'K': return ZenEvent_K; case 'L': return ZenEvent_L; case 'M': return ZenEvent_M; case 'N': return ZenEvent_N; case 'O': return ZenEvent_O;
//         case 'P': return ZenEvent_P; case 'Q': return ZenEvent_Q; case 'R': return ZenEvent_R; case 'S': return ZenEvent_S; case 'T': return ZenEvent_T;
//         case 'U': return ZenEvent_U; case 'V': return ZenEvent_V; case 'W': return ZenEvent_W; case 'X': return ZenEvent_X; case 'Y': return ZenEvent_Y;
//         case 'Z': return ZenEvent_Z;

//         case VK_F1: return ZenEvent_F1; case VK_F2:  return ZenEvent_F2;  case VK_F3:  return ZenEvent_F3;  case VK_F4:  return ZenEvent_F4;
//         case VK_F5: return ZenEvent_F5; case VK_F6:  return ZenEvent_F6;  case VK_F7:  return ZenEvent_F7;  case VK_F8:  return ZenEvent_F8;
//         case VK_F9: return ZenEvent_F9; case VK_F10: return ZenEvent_F10; case VK_F11: return ZenEvent_F11; case VK_F12: return ZenEvent_F12;

//         case VK_NUMPAD0: return ZenEvent_Keypad0; case VK_NUMPAD1: return ZenEvent_Keypad1;
//         case VK_NUMPAD2: return ZenEvent_Keypad2; case VK_NUMPAD3: return ZenEvent_Keypad3;
//         case VK_NUMPAD4: return ZenEvent_Keypad4; case VK_NUMPAD5: return ZenEvent_Keypad5;
//         case VK_NUMPAD6: return ZenEvent_Keypad6; case VK_NUMPAD7: return ZenEvent_Keypad7;
//         case VK_NUMPAD8: return ZenEvent_Keypad8; case VK_NUMPAD9: return ZenEvent_Keypad9;

//         case VK_LEFT:       return ZenEvent_LeftArrow;
//         case VK_RIGHT:      return ZenEvent_RightArrow;
//         case VK_UP:         return ZenEvent_UpArrow;
//         case VK_DOWN:       return ZenEvent_DownArrow;

//         case VK_TAB:        return ZenEvent_Tab;

//         case VK_PRIOR:      return ZenEvent_PageUp;
//         case VK_NEXT:       return ZenEvent_PageDown;
//         case VK_HOME:       return ZenEvent_Home;
//         case VK_END:        return ZenEvent_End;
//         case VK_INSERT:     return ZenEvent_Insert;
//         case VK_DELETE:     return ZenEvent_Delete;
//         case VK_BACK:       return ZenEvent_Backspace;
//         case VK_SPACE:      return ZenEvent_Space;
//         case VK_RETURN:     return ZenEvent_Enter;
//         case VK_ESCAPE:     return ZenEvent_Escape;
//         case VK_OEM_7:      return ZenEvent_Apostrophe;
//         case VK_OEM_COMMA:  return ZenEvent_Comma;
//         case VK_OEM_MINUS:  return ZenEvent_Minus;
//         case VK_OEM_PERIOD: return ZenEvent_Period;
//         case VK_OEM_2:      return ZenEvent_Slash;
//         case VK_OEM_1:      return ZenEvent_Semicolon;
//         case VK_OEM_PLUS:   return ZenEvent_Equal;
//         case VK_OEM_4:      return ZenEvent_LeftBracket;
//         case VK_OEM_5:      return ZenEvent_Backslash;
//         case VK_OEM_6:      return ZenEvent_RightBracket;
//         case VK_OEM_3:      return ZenEvent_GraveAccent;
//         case VK_CAPITAL:    return ZenEvent_CapsLock;
//         case VK_SCROLL:     return ZenEvent_ScrollLock;
//         case VK_NUMLOCK:    return ZenEvent_NumLock;
//         case VK_SNAPSHOT:   return ZenEvent_PrintScreen;
//         case VK_PAUSE:      return ZenEvent_Pause;

//         case VK_DECIMAL:  return ZenEvent_KeypadDecimal;
//         case VK_DIVIDE:   return ZenEvent_KeypadDivide;
//         case VK_MULTIPLY: return ZenEvent_KeypadMultiply;
//         case VK_SUBTRACT: return ZenEvent_KeypadSubtract;
//         case VK_ADD:      return ZenEvent_KeypadAdd;

//         case VK_RETURN + 0x100: return ZenEvent_KeypadEnter; // Todo: see glw3 how they did it
//         case VK_LSHIFT:         return ZenEvent_LeftShift;
//         case VK_LCONTROL:       return ZenEvent_LeftCtrl;
//         case VK_LMENU:          return ZenEvent_LeftAlt;
//         case VK_LWIN:           return ZenEvent_LeftSuper;
//         case VK_RSHIFT:         return ZenEvent_RightShift;
//         case VK_RCONTROL:       return ZenEvent_RightCtrl;
//         case VK_RMENU: return ZenEvent_RightAlt;
//         case VK_RWIN:  return ZenEvent_RightSuper;
//         case VK_APPS:  return ZenEvent_Menu;
//       }
//     } break;
//   }

//   return ZenEvent_None;
// }

// // REVISE(RJ):
// ZEN_FUNCTION void
// ZenAcquireConsoleWindow(ZenWindow *Window)
// { if(Window)
//   {
//     AllocConsole();
//     Window->NativeHandle = GetConsoleWindow();
//   }
// }

// ZEN_FUNCTION i32
// ZenIsWindowValid(ZenWindow *Window)
// {
//   return (Window) && IsWindow((HWND) Window->NativeHandle);
// }

// ZEN_FUNCTION i32
// ZenIsWindowInForeground(ZenWindow *Window)
// {
//   return (Window) && (GetForegroundWindow() == (HWND) Window->NativeHandle);
// }

// ZEN_FUNCTION VOID
// ZenShowWindow(ZenWindow *Window)
// { if((Window)&&(!Window->Visible))
//   {
//     ShowWindow((HWND) Window->NativeHandle, SW_SHOW);
//     Window->Visible=1;
//   }
// }

// ZEN_FUNCTION i32x2
// ZenGetWindowTrimFromClient(i32x2 ClientArea)
// { RECT Client;
//   Client.left   = 0; Client.right  = ClientArea.X;
//   Client.top    = 0; Client.bottom = ClientArea.Y;
//   if(AdjustWindowRect(&Client, ZEN_DEFAULT_WINDOW_STYLE_WIN32, FALSE))
//   { ClientArea.X = Client.right - Client.left;
//     ClientArea.Y = Client.bottom - Client.top;
//   }
//   return ClientArea;
// }

// ZEN_FUNCTION void
// ZenPullWindowMeasure(ZenWindow *Window)
// { // PENDING(RJ): WPF_ASYNCWINDOWPLACEMENT
//   WINDOWPLACEMENT Placement = { sizeof(WINDOWPLACEMENT) };

//   if(GetWindowPlacement((HWND) Window->NativeHandle, & Placement))
//   { Window->Location.Min.X = Placement.rcNormalPosition.left;
//     Window->Location.Min.Y = Placement.rcNormalPosition.top;
//   }

//   // TODO(RJ): PRETTY SURE GET_WINDOW_PLACEMENT DOES THIS AS WELL
//   RECT Client;
//   if(GetClientRect((HWND) Window->NativeHandle, & Client))
//   { Window->Location.Max.X = Client.right - Client.left;
//     Window->Location.Max.Y = Client.bottom - Client.top;
//   }
// }

// #if 0
// ZEN_FUNCTION i32
// ZenSetNativeWindowPositionAndClientSize(void *NativeWindow, i32x4 Area)
// { DWORD Flags = SWP_NOACTIVATE | SWP_FRAMECHANGED;

//   if((Area.Min.X|Area.Min.Y) == -1) Flags |= SWP_NOMOVE;

//   if((Area.Max.X|Area.Max.Y) != -1)
//   { RECT WindowRect = { 0, 0, Area.Max.X, Area.Max.Y };
//     if(AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE))
//     { Area.Max.X = WindowRect.right - WindowRect.left;
//       Area.Max.Y = WindowRect.bottom - WindowRect.top;
//     }
//   } else Flags |= SWP_NOSIZE;

//   if(SetWindowPos((HWND) NativeWindow, HWND_NOTOPMOST, Area.Min.X, Area.Min.Y, Area.Max.X, Area.Max.Y, Flags))
//   { return TRUE;
//   }
//   return FALSE;
// }
// #endif

// ZEN_FUNCTION i32
// ZenWindowReceivedMessage_win32(ZenWindow *Window, UINT Message, WPARAM wParam, LPARAM lParam)
// { switch(Message)
//   { // NOTE(RJ): A Brief Note On The Fiber Mechanism:
//     //
//     // When the window is being moved or resized an event is sent to the window and or the message queue of the thread
//     // that created the window.
//     //
//     // So long as the user keeps moving the window or resizing it, we will never exit the message handling loop - since
//     // we get a message every time the user moves or resizes the window - causing the game side of things to freeze.
//     //
//     // To prevent this from happening, we start a timer to switch between game code and message handling as soon we we detect
//     // the start of a continuous event loop,
//     //
//     // This may become obsolete if the game were ever to run on a background thread instead.
//     //
//     // TODO(RJ):
//     // Fibers are not working 100% for me, you can still freeze the window momentarily when you press the window border.
//     //
//     // TODO(RJ):
//     // Use GetCapture() and SetCapture(HWND)
//     case WM_PAINT:
//     case WM_TIMER:
//     { SwitchToFiber(Window->ControlFiber);
//     } break;
//     case WM_ENTERSIZEMOVE:
//     case WM_ENTERMENULOOP:
//     { SetTimer((HWND) Window->NativeHandle, 0, 1, 0);
//     } break;
//     case WM_EXITSIZEMOVE:
//     case WM_EXITMENULOOP:
//     { KillTimer((HWND) Window->NativeHandle, 0);
//     } break;
//     case WM_CLOSE:
//     case WM_QUIT:
//     { TRACE_I("Quitted...");
//       PostQuitMessage(0);
//       Window->Quitted = TRUE;
//     } break;
//     case WM_MOUSEMOVE:
//     { // TODO(RJ): is this the best way to query mouse delta?
//       i32x4 WindowQuad = Window->Location;
//       i32x2 WindowSize = WindowQuad.Max;
//       if( (WindowSize.X != 0.f) &&
//           (WindowSize.Y != 0.f) )
//       {
//         f32x2 InvWindowSize = 1.f / WindowSize; // TODO(RJ): CPP

//         f32x2 Half = {.5f, .5f };

//         i32x2 Cursor;
//         Cursor.X = GET_X_LPARAM(lParam);
//         Cursor.Y = GET_Y_LPARAM(lParam);

//         f32x2 Normalized = Cursor * InvWindowSize - Half; // TODO(RJ): CPP

//         f32x2 Inverted = Normalized * f32x2_make(1.f, -1.f); // TODO(RJ): CPP

//         ZenEvent *Event = Window->Event + ZenEvent_MouseMove;
//         Event->IsEvent  = TRUE;
//         Event->Cursor   = Inverted;
//         Event->Counter  ++;

//       } else( TRACE_W("invalid window size") );
//     } break;
//     // TODO(RJ): Clean up!
//     case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
//     case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
//     case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
//     case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
//     { ZenEvent *Event = Window->Event + ZenInternalizeWindowEvent_win32(Message,wParam);
//       AssertW(~Event->Counter & 0x01, L"invalid state %i", Event->Counter);
//       Event->IsEvent = TRUE;
//       Event->Counter ++; // Odd

//       if(!Window->ClickFocused)
//       { Window->ClickFocused = TRUE;
//         SetCapture((HWND) Window->NativeHandle);
//       }
//     } break;
//     case WM_LBUTTONUP: case WM_RBUTTONUP:
//     case WM_MBUTTONUP: case WM_XBUTTONUP:
//     { ZenEvent *Event = Window->Event + ZenInternalizeWindowEvent_win32(Message,wParam);
//       AssertW(Event->Counter & 0x01, L"invalid state %i", Event->Counter);
//       Event->Counter --; // Even
//       Event->IsEvent = TRUE;

//       AssertW(Window->ClickFocused, L"invalid state, window should have been click focused");
//       Window->ClickFocused = FALSE;
//       ReleaseCapture();
//     } break;
//     case WM_KEYUP:
//     case WM_KEYDOWN:
//     { i16 IsDown = Message == WM_KEYDOWN;
//       ZenEvent *Event = Window->Event + ZenInternalizeWindowEvent_win32(Message,wParam);
//       Event->Value   = (u16) wParam; // TODO(RJ): REVISE
//       Event->IsEvent = TRUE;
//       Event->Counter += IsDown*2-1;
//     } break;
//     default:
//     { return FALSE;
//     } break;
//   }
//   return TRUE;
// }

// ZEN_FUNCTION LRESULT CALLBACK
// ZenWindowCallback_win32(HWND NativeWindow,UINT Message,WPARAM wParam,LPARAM lParam)
// { ZenWindow *Window;
//   Window=(ZenWindow *)GetWindowLongPtrA(NativeWindow,GWLP_USERDATA);

//   if(Window&&ZenWindowReceivedMessage_win32(Window,Message,wParam,lParam))
//   {
//     // TODO(RJ):
//   }

//   return DefWindowProcW(NativeWindow,Message,wParam,lParam);
// }

// ZEN_FUNCTION VOID
// ZenWindowMessageGobbler_win32(ZenWindow *Window)
// { MSG M;
//   while(PeekMessage(&M, NULL, 0, 0, PM_REMOVE))
//   { TranslateMessage(&M);
//     if(!ZenWindowReceivedMessage_win32(Window, M.message, M.wParam, M.lParam))
//     { DispatchMessageW(&M);
//     }
//   }
// }

// ZEN_FUNCTION VOID
// ZenWindowMessageFiber_win32(LPVOID lpWindow)
// { ZenWindow *Window;
//   Window=(ZenWindow *)lpWindow;

//   for(;;)
//   { ZenWindowMessageGobbler_win32(Window);
//     SwitchToFiber(Window->ControlFiber);
//   }
// }


// ZEN_FUNCTION i32
// ZenInitExternalWindow(ZenExternalCore *Core, ZenWindow *Window, void *ControlFiber, ConstStrW WindowTitle)
// {
//   ZeroMemory(Window, sizeof(* Window));

//   ConstStrW WindowClassName;
//   WindowClassName=FormatW(L"ZenWindow::%s", WindowTitle);

//   WNDCLASSW WindowClass;
//   ZeroMemory(&WindowClass, sizeof(WindowClass));

//   WindowClass.lpfnWndProc   = ZenWindowCallback_win32;
//   WindowClass.hInstance     = GetModuleHandleW(NULL);
//   WindowClass.lpszClassName = WindowClassName;

//   if(RegisterClassW(&WindowClass))
//   {
// #if 0
//     i32x2 WindowSize = ZenGetWindowTrimFromClient(Location.Max);
// #endif
//     // Todo: we're not done with this ...
//     Window->NativeHandle = CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP, WindowClass.lpszClassName, WindowTitle,
//       ZEN_DEFAULT_WINDOW_STYLE_WIN32, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
//         NULL, NULL, WindowClass.hInstance, NULL);

//     if(ZenIsWindowValid(Window))
//     {
//       ZenPullWindowMeasure(Window);

//       Window->ControlFiber = ControlFiber;
//       Window->MessageFiber = CreateFiber(0, ZenWindowMessageFiber_win32, Window);

//       if(! Window->MessageFiber)
//       { TRACE_W("CreateFiber() [FAILED]");
//       }

//       // if(! SetWindowPos(Window->Window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE))
//       // { TRACE_W("SetWindowPos() [FAILED]");
//       // }
//       SetLastError(S_OK);
//       if(SetWindowLongPtrA((HWND) Window->NativeHandle, GWLP_USERDATA, (LONG_PTR) Window))
//       { TRACE_F("SetWindowLongPtrA() [FAILED], %s", GetLastErrorMSGW());
//       }
//       SetLastError(S_OK);
//       if(! GetWindowLongPtrA((HWND) Window->NativeHandle, GWLP_USERDATA))
//       { TRACE_F("GetWindowLongPtrA() [FAILED]");
//       }
//       return TRUE;
//     } else( TRACE_E("failed to create window") );
//   } else( TRACE_E("failed to register window class") );
//   return FALSE;
// }

// // Todo: speed ...
// ZEN_FUNCTION void
// ZenCorePushExternalWindow(ZenCore *Core, ZenWindow *Window)
// { for(i32 I = 0; I < ARRAYSIZE(Window->Event); ++ I)
//   { Window->Event[I].IsEvent = FALSE;
//   }
// }


// ZEN_FUNCTION i32
// ZenCorePullExternalWindow(ZenCore *Core, ZenWindow *Window)
// { ZenPullWindowMeasure(Window);
//   if(Window->MessageFiber)
//   { SwitchToFiber(Window->MessageFiber);
//   } else
//   { ZenWindowMessageGobbler_win32(Window);
//   }
//   return !Window->Quitted;
// }

// #endif // #ifndef _ZEN_WIN
// #endif // #ifdef _WIN