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
#include <ppltasks.h>

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

		bool Intersect(const Rect& other) const noexcept
		{
			auto distX = std::fabs(Center.x - other.Center.x);
			auto distY = std::fabs(Center.y - other.Center.y);

			return distX < (Width + other.Width) / 2 && distY < (Height + other.Height);
		}

		DirectX::XMFLOAT2 get_Center() const noexcept { return{ LeftTop.x + Width / 2, RightBottom.y + Height / 2 }; }
		DEFINE_PROPERTY_GET(Center, DirectX::XMFLOAT2);

		float get_Width() const noexcept { return RightBottom.x - LeftTop.x; }
		void set_Width(float value) noexcept { RightBottom.x = LeftTop.x + value; }
		DEFINE_PROPERTY(Width, float);

		float get_Height() const noexcept { return LeftTop.y - RightBottom.y; }
		void set_Height(float value) noexcept { LeftTop.y = RightBottom.y + value; }
		DEFINE_PROPERTY(Height, float);

		void Offset(float x, float y) noexcept
		{
			LeftTop.x += x; RightBottom.x += x;
			LeftTop.y += y; RightBottom.y += y;
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
		bool Sealed = false;

		QuadTreeNode()
		{
			NodeManager.SetNode(*this);
		}

		QuadTreeNode(const Rect& boundingRect)
			:QuadTreeNode(), BoundingRect(boundingRect)
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
			if (!Sealed && ContinueDivideFn()(BoundingRect, Level + 1))
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

		template<typename TFn>
		void ForEach(TFn kernel)
		{
			kernel(*this);
			if (Children)
				for (auto&& child : *Children)
					child.ForEach(std::forward<TFn>(kernel));
		}

		template<typename TFn>
		concurrency::task<void> ForEachAsync(TFn kernel)
		{
			co_await kernel(*this);
			if (Children)
				for (auto&& child : *Children)
					co_await child.ForEachAsync(std::forward<TFn>(kernel));
		}

		void HitTest(std::vector<QuadTreeNode*>& nodes, const Rect& rect)
		{
			if (BoundingRect.Intersect(rect))
				nodes.emplace_back(this);
			if (Children)
				for (auto&& child : *Children)
					child.HitTest(nodes, rect);
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

				for (auto&& child : *Children)
				{
					child.Level = Level + 1;
					child.NodeManager.SetNode(child);
				}
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