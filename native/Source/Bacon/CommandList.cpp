#include "Bacon.h"
#include "BaconInternal.h"

int Bacon_ExecuteCommands(int* commands, int commandCount, float* data, int dataCount)
{
	int* startCommands = commands;
	float* startData = data;
	for (; commands < startCommands + commandCount; )
	{
		switch (*commands++)
		{
			case Bacon_Command_PushTransform:
				Bacon_PushTransform();
				break;
			case Bacon_Command_PopTransform:
				Bacon_PopTransform();
				break;
			case Bacon_Command_Translate:
			{
				float x = *data++;
				float y = *data++;
				Bacon_Translate(x, y);
				break;
			}
			case Bacon_Command_Scale:
			{
				float sx = *data++;
				float sy = *data++;
				Bacon_Scale(sx, sy);
				break;
			}
			case Bacon_Command_Rotate:
			{
				Bacon_Rotate(*data++);
				break;
			}
			case Bacon_Command_SetTransform:
			{
				Bacon_SetTransform(data);
				data += 16;
				break;
			}
			case Bacon_Command_PushColor:
			{
				Bacon_PushColor();
				break;
			}
			case Bacon_Command_PopColor:
			{
				Bacon_PopColor();
				break;
			}
			case Bacon_Command_SetColor:
			{
				float r = *data++;
				float g = *data++;
				float b = *data++;
				float a = *data++;
				Bacon_SetColor(r, g, b, a);
				break;
			}
			case Bacon_Command_MultiplyColor:
			{
				float r = *data++;
				float g = *data++;
				float b = *data++;
				float a = *data++;
				Bacon_MultiplyColor(r, g, b, a);
				break;
			}
			case Bacon_Command_SetBlending:
			{
				int src = *commands++;
				int dest = *commands++;
				Bacon_SetBlending(src, dest);
				break;
			}
			case Bacon_Command_DrawImage:
			{
				
				float x1 = *data++;
				float y1 = *data++;
				float x2 = *data++;
				float y2 = *data++;
				Bacon_DrawImage(*commands++, x1, y1, x2, y2);
				break;
			}
			case Bacon_Command_DrawImageRegion:
			{
				float x1 = *data++;
				float y1 = *data++;
				float x2 = *data++;
				float y2 = *data++;
				float ix1 = *data++;
				float iy1 = *data++;
				float ix2 = *data++;
				float iy2 = *data++;
				Bacon_DrawImageRegion(*commands++, x1, y1, x2, y2, ix1, iy1, ix2, iy2);
				break;
			}
			case Bacon_Command_DrawImageQuad:
			{
				float* positions = data;
				float* texCoords = &data[4];
				float* colors = &data[8];
				data += 12;
				Bacon_DrawImageQuad(*commands++, positions, texCoords, colors);
				break;
			}
			case Bacon_Command_DrawLine:
			{
				float x1 = *data++;
				float y1 = *data++;
				float x2 = *data++;
				float y2 = *data++;
				Bacon_DrawLine(x1, y1, x2, y2);
				break;
			}
			case Bacon_Command_DrawTriangle:
			{
				float x1 = *data++;
				float y1 = *data++;
				float x2 = *data++;
				float y2 = *data++;
				float x3 = *data++;
				float y3 = *data++;
				Bacon_DrawTriangle(x1, y1, x2, y2, x3, y3);
				break;
			}
			case Bacon_Command_FillTriangle:
			{
				float x1 = *data++;
				float y1 = *data++;
				float x2 = *data++;
				float y2 = *data++;
				float x3 = *data++;
				float y3 = *data++;
				Bacon_FillTriangle(x1, y1, x2, y2, x3, y3);
				break;
			}
			case Bacon_Command_DrawRect:
			{
				float x1 = *data++;
				float y1 = *data++;
				float x2 = *data++;
				float y2 = *data++;
				Bacon_DrawRect(x1, y1, x2, y2);
				break;
			}
			case Bacon_Command_FillRect:
			{
				float x1 = *data++;
				float y1 = *data++;
				float x2 = *data++;
				float y2 = *data++;
				Bacon_FillRect(x1, y1, x2, y2);
				break;
			}
			case Bacon_Command_SetShaderUniformFloats:
			{
				int handle = *commands++;
				int uniform = *commands++;
				int size = *commands++;
				float* value = data;
				data += size;
				Bacon_SetShaderUniform(handle, uniform, value, size * 4);
				break;
			}
			case Bacon_Command_SetShaderUniformInts:
			{
				int handle = *commands++;
				int uniform = *commands++;
				int size = *commands++;
				int* value = commands;
				commands += size;
				Bacon_SetShaderUniform(handle, uniform, value, size * 4);
				break;
			}
			case Bacon_Command_SetSharedShaderUniformFloats:
			{
				int handle = *commands++;
				int size = *commands++;
				float* value = data;
				data += size;
				Bacon_SetSharedShaderUniform(handle, value, size * 4);
				break;
			}
			case Bacon_Command_SetSharedShaderUniformInts:
			{
				int handle = *commands++;
				int size = *commands++;
				int* value = commands;
				commands += size;
				Bacon_SetSharedShaderUniform(handle, value, size * 4);
				break;
			}
			case Bacon_Command_SetShader:
			{
				int handle = *commands++;
				Bacon_SetShader(handle);
				break;
			}
			case Bacon_Command_Clear:
			{
				float r = *data++;
				float g = *data++;
				float b = *data++;
				float a = *data++;
				Bacon_Clear(r, g, b, a);
				break;
			}
			case Bacon_Command_SetFrameBuffer:
			{
				int handle = *commands++;
				float contentScale = *data++;
				Bacon_SetFrameBuffer(handle, contentScale);
				break;
			}
			case Bacon_Command_SetViewport:
			{
				int x = *commands++;
				int y = *commands++;
				int width = *commands++;
				int height = *commands++;
				float contentScale = *data++;
				Bacon_SetViewport(x, y, width, height, contentScale);
				break;
			}
			default:
				return Bacon_Error_InvalidArgument;
		}
	}
	
	if ((data - startData) != dataCount)
		return Bacon_Error_InvalidArgument;
	
	return Bacon_Error_None;
}