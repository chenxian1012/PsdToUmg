
#pragma once

#include "PSDParser.Base.h"
#include <stdint.h>

namespace PSDParser
{
	enum class LayerObjectType : int32_t
	{
		Image,
		Button,
		ProgressBar,
		Text,
	};

	enum class LayerAdditionalObjectType : int32_t
	{
		None,
		Normal,
		Pressed,
		Hovered,
		Background,
		Fill,
		Marquee,
	};

	struct Rect
	{
		int32_t Top;
		int32_t Left;
		int32_t Bottom;
		int32_t Right;
	};

	class Layer
	{
		friend class Document;

        std::u16string	name_;
		std::vector<uint8_t>		data_;
		Rect						area_;
		bool						isFolderBegin_;
        bool						isFolderEnd_;
        std::u16string	text_;
		int opacity_ = 255;
	public:

#ifndef SWIG
        Layer(const std::vector<uint8_t>& data, Rect area, bool isFolderBegin, bool isFolderEnd, std::u16string name, int opacity);
#endif
        Layer() = default;
        virtual ~Layer() = default;

        void Extend(Rect newArea);

		const void* GetData()
		{
			return data_.data();
		}

		Rect GetRect()
		{
			return area_;
		}

		bool IsFolderBegin() const { return isFolderBegin_; }

		bool IsFolderEnd() const { return isFolderEnd_; }

		const uchar* GetName() const { return name_.c_str(); }

		const uchar* GetText() const { return text_.c_str(); }

		int GetOpacity() const { return opacity_; }

		LayerObjectType	ObjectType = LayerObjectType::Image;

		LayerAdditionalObjectType AdditionalObjectType = LayerAdditionalObjectType::None;
	};

	class Document
	{
	private:

		std::vector<std::shared_ptr<Layer>>	layers_;
		int32_t						colorDepth_ = 0;
		int32_t						docWidth_ = 0;
		int32_t						docHeight_ = 0;

	public:

		Document();

		virtual ~Document();

#ifndef SWIG
		bool Load(const void* data, int32_t size);
#endif

		static std::shared_ptr<Document> Create(const void* data, int32_t size);

		int32_t GetLayerCount();

		std::shared_ptr<Layer> GetLayer(int32_t index);

		int32_t GetWidth() const;

		int32_t GetHeight() const;

		int32_t GetDepth() const;
	};
}

