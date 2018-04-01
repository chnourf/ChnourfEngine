#include "../Dependencies/glew/glew.h"
#include "WorldGridGeneratorDebug.h"

#include "../Core/Vector.h"
#include <algorithm>
#include <future>
#include <vector>
#include "../Dependencies/SOIL/SOIL.h"
#include "../WorldGenerator/TerrainGenerationFunctions.h"
#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/GLFW/glfw3.h"
#include "../Rendering/Camera.h"
#include "../Core/Math.h"

#include <iostream>

namespace MapDebug
{
	const unsigned int pictureDimension{ 512 };// TerrainGeneration::GetMapTileAmount() };
	static const unsigned int MetersPerPixel{ unsigned(TerrainGeneration::GetMapSize()) / pictureDimension };
	static const auto locWaterCol = vec3i(37, 125, 177);
	static const auto locWaterColFloat = vec3f(locWaterCol.x / 255.f, locWaterCol.y / 255.f, locWaterCol.z / 255.f);
	static const int locNumThreads = 4;
	static const auto midPos = Vector2<float>(TerrainGeneration::GetMapSize() / 2.f, TerrainGeneration::GetMapSize() / 2.f);
	static GLuint locDebugMap;
	static const unsigned debugMapAndMinimapSize = 256u;
	static vec2f locFacingDirection;
	static const int viewTriangleLength = 30;

	void SetPixel(std::vector<unsigned char>& anImage, const vec2i& pixelPosition, const vec3i& aColor)
	{
		auto id = 4 * (pixelPosition.x + pictureDimension * pixelPosition.y);
		auto it = anImage.begin() + id;
		*it = aColor.x; ++it;
		*it = aColor.y; ++it;
		*it = aColor.z; ++it;
	}

	vec2i PositionToPixel(vec2f aPostion)
	{
		unsigned int xPixel = (unsigned int)aPostion.x / MetersPerPixel;
		unsigned int yPixel = (unsigned int)aPostion.y / MetersPerPixel;
		if (yPixel == pictureDimension)
		{
			yPixel -= 1u;
		}
		if (xPixel == pictureDimension)
		{
			xPixel -= 1u;
		}

		return vec2i(xPixel, yPixel);
	}

	vec3f DeduceBiomeColor(TerrainGeneration::Biome aBiome)
	{
		switch (aBiome)
		{
		case TerrainGeneration::Biome::Snow:
			return vec3f(1.f, 1.f, 1.f);
			break;
		case TerrainGeneration::Biome::Tundra:
			return vec3f(221.f/255.f, 221.f / 255.f, 187.f / 255.f);
			break;
		case TerrainGeneration::Biome::Bare:
			return vec3f(187.f / 255.f, 187.f / 255.f, 187.f / 255.f);
			break;
		case TerrainGeneration::Biome::Scorched:
			return vec3f(153.f / 255.f, 153.f / 255.f, 153.f / 255.f);
			break;
		case TerrainGeneration::Biome::Taiga:
			return vec3f(204.f / 255.f, 212.f / 255.f, 187.f / 255.f);
			break;
		case TerrainGeneration::Biome::Shrubland:
			return vec3f(196.f / 255.f, 204.f / 255.f, 187.f / 255.f);
			break;
		case TerrainGeneration::Biome::TemperateDesert:
			return vec3f(208.f / 255.f, 252.f / 255.f, 182.f / 255.f);
			break;
		case TerrainGeneration::Biome::TemperateRainForest:
			return vec3f(159.f / 255.f, 1.f, 144.f / 255.f);
			break;
		case TerrainGeneration::Biome::TemperateDeciduousForest:
			return vec3f(160.f / 255.f, 181.f / 255.f, 149.f / 255.f);
			break;
		case TerrainGeneration::Biome::Grassland:
			return vec3f(162.f / 255.f, 179.f / 255.f, 104.f / 255.f);
			break;
		case TerrainGeneration::Biome::TropicalRainForest:
			return vec3f(162.f / 255.f, 229.f / 255.f, 96.f / 255.f);
			break;
		case TerrainGeneration::Biome::SubtropicalDesert:
			return vec3f(1.f, 187.f / 255.f, 28.f / 255.f);
			break;
		case TerrainGeneration::Biome::Sea:
			return locWaterColFloat;
			break;
		default:
			return vec3f(0.f);
			break;
		}
	}

	void DrawLine(std::vector<unsigned char>& anImage, const vec2i& aStart, const vec2i& aEnd, const vec3i& aCol)
	{
		unsigned int distance = Vector2<int>::Distance(aStart, aEnd);
		for (unsigned int i = 0; i <= distance; ++i)
		{
			float r = (float)i / (float)distance;
			auto point = Vector2<int>(glm::mix(aStart.x, aEnd.x, r), glm::mix(aStart.y, aEnd.y, r));

			SetPixel(anImage, point, aCol);
		}
	}

	void DrawTriangle(std::vector<unsigned char>& anImage, const TerrainGeneration::Triangle& triangle, vec3i color)
	{
		auto& a = *triangle.myA;
		auto& b = *triangle.myB;
		auto& c = *triangle.myC;

		auto atob = Vector2<float>(b.myPosition.x - a.myPosition.x, b.myPosition.y - a.myPosition.y);
		auto btoc = Vector2<float>(c.myPosition.x - b.myPosition.x, c.myPosition.y - b.myPosition.y);
		auto ctoa = Vector2<float>(a.myPosition.x - c.myPosition.x, a.myPosition.y - c.myPosition.y);

		Vector2<float> maxBound;
		maxBound.x = std::max(a.myPosition.x, std::max(b.myPosition.x, c.myPosition.x));
		maxBound.y = std::max(a.myPosition.y, std::max(b.myPosition.y, c.myPosition.y));
		Vector2<float> minBound;
		minBound.x = std::min(a.myPosition.x, std::min(b.myPosition.x, c.myPosition.x));
		minBound.y = std::min(a.myPosition.y, std::min(b.myPosition.y, c.myPosition.y));

		for (float i = minBound.x; i < maxBound.x; i += (float)MetersPerPixel / 2.f)
		{
			for (float j = minBound.y; j < maxBound.y; j += (float)MetersPerPixel / 2.f)
			{
				Vector2<float> atoPixel = Vector2<float>(i - a.myPosition.x, j - a.myPosition.y);
				Vector2<float> btoPixel = Vector2<float>(i - b.myPosition.x, j - b.myPosition.y);
				Vector2<float> ctoPixel = Vector2<float>(i - c.myPosition.x, j - c.myPosition.y);
				bool isInTriangle = (Vector2<float>::Cross(atob, atoPixel) >= 0.f && Vector2<float>::Cross(btoc, btoPixel) >= 0.f && Vector2<float>::Cross(ctoa, ctoPixel) >= 0.f);

				if (isInTriangle)
				{
					SetPixel(anImage, PositionToPixel(vec2f(i, j)), color);
				}
			}
		}
	}

	void DrawRiver(std::vector<unsigned char>& anImage, std::vector<TerrainGeneration::Point*> aRiver)
	{
		for (auto it = aRiver.begin(); it < aRiver.end() - 1; ++it)
		{
			DrawLine(anImage,
				PositionToPixel((*it)->myPosition),
				PositionToPixel((*(it + 1))->myPosition),
				locWaterCol);
		}
	}

	void DrawAndSaveDebugImages(const TerrainGeneration::WorldGrid& aGrid)
	{
		std::chrono::time_point<std::chrono::system_clock> start, end;
		start = std::chrono::system_clock::now();

		std::vector<unsigned char> biomeImageData;
		biomeImageData.assign(pictureDimension * pictureDimension * 4, 255);
		std::vector<unsigned char> elevationImageData;
		elevationImageData.assign(pictureDimension * pictureDimension * 4, 255);
		std::vector<unsigned char> rainfallImageData;
		rainfallImageData.assign(pictureDimension * pictureDimension * 4, 255);
		std::vector<unsigned char> temperatureImageData;
		temperatureImageData.assign(pictureDimension * pictureDimension * 4, 255);

		std::vector<std::future<void>> handles;
		auto linesProcessedPerThread = pictureDimension / locNumThreads;
		auto currentMin = 0;

		for (auto thread = 0; thread < locNumThreads; thread++)
		{
			handles.push_back(std::async(std::launch::async, [currentMin, linesProcessedPerThread, &aGrid, &biomeImageData, &elevationImageData, &rainfallImageData, &temperatureImageData]() {
				for (int i = currentMin; i < currentMin + linesProcessedPerThread; ++i)
				{
					for (int j = 0; j < pictureDimension; ++j)
					{
						const float x = float(MetersPerPixel) * float(i - int(pictureDimension) / 2);
						const float y = float(MetersPerPixel) * float(j - int(pictureDimension) / 2);
						const auto elevation = TerrainGeneration::ComputeElevation(x, y, true);
						auto elevationCol = elevation < 0.f ? locWaterCol : vec3i(255 * 0.5f * (elevation / TerrainGeneration::GetMultiplier()));
						const auto temperature = TerrainGeneration::ComputeTemperature(x, elevation, y);
						const auto rainfall = aGrid.SampleGridRainfall(vec2f(x, y));
						auto tempCol = vec3i(200 * temperature, 0, 200 * (1.f - temperature));
						SetPixel(elevationImageData, vec2i(i, j), elevationCol);
						SetPixel(rainfallImageData, vec2i(i, j), vec3i(255 * rainfall));
						auto biomeCol = elevation < 0.f ? vec3f(locWaterCol.x / 255.f, locWaterCol.y / 255.f, locWaterCol.z / 255.f) : DeduceBiomeColor(TerrainGeneration::DeduceBiome(temperature, rainfall));
						SetPixel(biomeImageData, vec2i(i, j), vec3i(255 * biomeCol.x, 255 * biomeCol.y, 255 * biomeCol.z));
						SetPixel(temperatureImageData, vec2i(i, j), tempCol);
					}
				}
			}));

			currentMin += linesProcessedPerThread;
		}

		for (auto& handle : handles)
		{
			handle.get();
		}

		end = std::chrono::system_clock::now();

		int elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>
			(end - start).count();

		std::cout << "elapsed time: " << elapsed_seconds << "ms\n";

		for (const auto& river : aGrid.GetRivers())
		{
			DrawRiver(biomeImageData, river);
			DrawRiver(elevationImageData, river);
		}

		SOIL_save_image("biomes.bmp", SOIL_SAVE_TYPE_BMP,
			pictureDimension, pictureDimension, 4,
			&biomeImageData[0]);
		SOIL_save_image("rainfall.bmp", SOIL_SAVE_TYPE_BMP,
			pictureDimension, pictureDimension, 4,
			&rainfallImageData[0]);
		SOIL_save_image("temperature.bmp", SOIL_SAVE_TYPE_BMP,
			pictureDimension, pictureDimension, 4,
			&temperatureImageData[0]);
		SOIL_save_image("elevation.bmp", SOIL_SAVE_TYPE_BMP,
			pictureDimension, pictureDimension, 4,
			&elevationImageData[0]);
	}

	void CreateMinimapTexture()
	{
		glGenTextures(1, &locDebugMap);
		glBindTexture(GL_TEXTURE_2D, locDebugMap);
		int width, height;
		unsigned char* image = SOIL_load_image("biomes.bmp", &width, &height, 0, SOIL_LOAD_RGB);
		assert(image);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void SetFacingDirection(const vec2f& aFacingDirection)
	{
		locFacingDirection = aFacingDirection;
	}

	void RenderMinimap(const vec2f& aPosition)
	{
		//triangle avec fov

		ImGui::Begin("Debug Map");
		auto camPosRelativeToMap = 1.f / TerrainGeneration::GetMapSize() * (aPosition + midPos);
		ImGui::Image((void *)(intptr_t)locDebugMap, ImVec2(debugMapAndMinimapSize, debugMapAndMinimapSize),
			ImVec2(camPosRelativeToMap.x - 0.1f, camPosRelativeToMap.y - 0.1f),
			ImVec2(camPosRelativeToMap.x + 0.1f, camPosRelativeToMap.y + 0.1f));
		const auto debugMiniMapImageCenter = ImVec2((ImGui::GetItemRectMax().x + ImGui::GetItemRectMin().x) / 2,
			(ImGui::GetItemRectMax().y + ImGui::GetItemRectMin().y) / 2);

		auto aFacingDirectionRotated = vec2f(-locFacingDirection.y, locFacingDirection.x);

		const int triangleHalfWidth = viewTriangleLength * tan(float(Camera::ourFov * M_PI) / 180.f);
		const auto topLeftVisionTriangeCorner = ImVec2(debugMiniMapImageCenter.x + locFacingDirection.x * viewTriangleLength + aFacingDirectionRotated.x * triangleHalfWidth,
			debugMiniMapImageCenter.y + locFacingDirection.y * viewTriangleLength + aFacingDirectionRotated.y * triangleHalfWidth);
		const auto topRightVisionTriangeCorner = ImVec2(debugMiniMapImageCenter.x + locFacingDirection.x * viewTriangleLength - aFacingDirectionRotated.x * triangleHalfWidth,
			debugMiniMapImageCenter.y + locFacingDirection.y * viewTriangleLength - aFacingDirectionRotated.y * triangleHalfWidth);
		ImGui::GetWindowDrawList()->AddTriangleFilled(debugMiniMapImageCenter, topLeftVisionTriangeCorner, topRightVisionTriangeCorner, IM_COL32(100, 100, 255, 120));

		ImGui::GetWindowDrawList()->AddCircleFilled(debugMiniMapImageCenter, 4.f, IM_COL32(0, 0, 0, 255));
		vec2f adjustedPostion = aPosition + midPos;
		ImGui::Image((void *)(intptr_t)locDebugMap, ImVec2(debugMapAndMinimapSize, debugMapAndMinimapSize));
		const auto debugMapCamPos = ImVec2(ImGui::GetItemRectMin().x + camPosRelativeToMap.x * debugMapAndMinimapSize,
			ImGui::GetItemRectMin().y + camPosRelativeToMap.y * debugMapAndMinimapSize);
		ImGui::GetWindowDrawList()->AddCircleFilled(debugMapCamPos, 2.f, IM_COL32(0, 0, 0, 255));
		ImGui::Text("Element on Grid : x %d, y %d",
			int(adjustedPostion.x)*TerrainGeneration::GetMapTileAmount() / int(TerrainGeneration::GetMapSize()),
			int(adjustedPostion.y)*TerrainGeneration::GetMapTileAmount() / int(TerrainGeneration::GetMapSize()));
		ImGui::End();
	}

}