//
// Red Alert Plus Engine
// Collision
// 作者：SunnyCase
// 创建时间：2016-05-31
//
#pragma once
#include "../../include/common.h"
#include <DirectXMath.h>
#include <algorithm>
#include <vector>
#include <deque>
#include <memory>

namespace Tomato
{
	struct Rect
	{
		DirectX::XMFLOAT2 LeftTop = {}, RightBottom = {};

		Rect() {}

		Rect(const DirectX::XMFLOAT2& leftTop, const DirectX::XMFLOAT2& rightBottom)
			:LeftTop(leftTop), RightBottom(rightBottom) {}

		Rect(float left, float top, float right, float bottom)
			:LeftTop(left, top), RightBottom(right, bottom) {}

		bool Contains(const Rect& other) const noexcept
		{
			return LeftTop.x <= other.LeftTop.x && LeftTop.y >= other.LeftTop.y &&
				RightBottom.x >= other.RightBottom.x && RightBottom.y <= other.RightBottom.y;
		}

		float get_Width() const noexcept { return RightBottom.x - LeftTop.x; }
		void set_Width(float value) noexcept { RightBottom.x = LeftTop.x + value; }
		DEFINE_PROPERTY(Width, float);

		float get_Height() const noexcept { return LeftTop.y - RightBottom.y; }
		void set_Height(float value) noexcept { LeftTop.y = RightBottom.y + value; }
		DEFINE_PROPERTY(Height, float);

		void Offset(float x, float y) noexcept
		{
			LeftTop.x += x; RightBottom.x += x;
			LeftTop.y += x; RightBottom.y += y;
		}

		Rect MakeOffset(float x, float y) const noexcept
		{
			Rect rect(*this);
			rect.Offset(x, y);
			return rect;
		}

		Rect MakeOffset(const DirectX::XMFLOAT2& offset) const noexcept
		{
			return MakeOffset(offset.x, offset.y);
		}

		Rect& operator/=(float scale) noexcept
		{
			Width /= scale;
			Height /= scale;
			return *this;
		}

		Rect operator/(float scale) const noexcept
		{
			Rect rect(*this);
			rect /= scale;
			return rect;
		}
	};

	struct BoundingRect : public Rect
	{
		using Rect::Rect;

		void Add(const Rect& rect) noexcept
		{
			LeftTop.x = std::min(LeftTop.x, rect.LeftTop.x);
			LeftTop.y = std::max(LeftTop.y, rect.LeftTop.y);
			RightBottom.x = std::max(RightBottom.x, rect.RightBottom.x);
			RightBottom.y = std::min(RightBottom.y, rect.RightBottom.y);
		}
	};

	template<typename T, typename TNodeManager, typename ContinueDivideFn>
	struct QuadTreeNode
	{
		std::vector<T> Objects;
		Rect BoundingRect;
		std::unique_ptr<std::array<QuadTreeNode, 4>> Children;
		size_t Level = 0;
		TNodeManager NodeManager;

		QuadTreeNode() {}

		QuadTreeNode(const Rect& boundingRect)
			:BoundingRect(boundingRect)
		{

		}

		bool IsLeaf() const noexcept { return Children.empty(); }
		void Clear()
		{
			Children = nullptr;
			Objects.clear();
		}

		void Add(const T& obj, const Tomato::Rect& rect)
		{
			if (ContinueDivideFn()(BoundingRect, Level))
			{
				InitializeRect();
				for (auto&& child : *Children)
				{
					if (child.BoundingRect.Contains(rect))
					{
						child.Add(obj, rect);
						return;
					}
				}
			}
			Objects.emplace_back(obj);
		}
	private:
		void InitializeRect()
		{
			if (!Children)
			{
				auto trunkRect = BoundingRect / 2;
				Children = std::make_unique<std::array<QuadTreeNode, 4>>();
				Children->at(0).BoundingRect = trunkRect;
				Children->at(1).BoundingRect = trunkRect.MakeOffset(trunkRect.Width, 0);
				Children->at(2).BoundingRect = trunkRect.MakeOffset(trunkRect.Width, trunkRect.Height);
				Children->at(3).BoundingRect = trunkRect.MakeOffset(0, trunkRect.Height);

				Children->at(0).Level =
					Children->at(1).Level =
					Children->at(2).Level =
					Children->at(3).Level = Level + 1;
			}
		}
	};

	struct DefaultContinueDivideFn
	{
		bool operator()(const Rect&, size_t) const noexcept
		{
			return true;
		}
	};

	template<typename T, typename TNodeManager, typename ContinueDivideFn = DefaultContinueDivideFn>
	using QuadTree = QuadTreeNode<T, TNodeManager, ContinueDivideFn>;
}