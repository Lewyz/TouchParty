#ifndef TOUCHPARTY_UISCROLLCONTAINER_H
#define TOUCHPARTY_UISCROLLCONTAINER_H

#include <algorithm>
#include <cmath>
#include "Shader.h"
#include "UIDrawHelpers.h"
#include "UITheme.h"

class UIScrollContainer {
public:
    UIScrollContainer()
        : scrollOffset_(0.0f),
          maxScrollOffset_(0.0f),
          viewportHeight_(0.0f),
          contentHeight_(0.0f),
          isDragging_(false),
          draggedThresholdExceeded_(false),
          startY_(0.0f),
          startOffset_(0.0f),
          totalDragDistance_(0.0f) {}

    void setContentAndViewportHeight(float contentHeight, float viewportHeight) {
        viewportHeight_ = viewportHeight;
        contentHeight_ = contentHeight;
        maxScrollOffset_ = std::max(0.0f, contentHeight - viewportHeight);
        scrollOffset_ = std::clamp(scrollOffset_, 0.0f, maxScrollOffset_);
    }

    void onTouchDown(float normX, float normY, float boundsX, float boundsY, float boundsW, float boundsH) {
        if (normX >= boundsX - boundsW * 0.5f && normX <= boundsX + boundsW * 0.5f &&
            normY >= boundsY - boundsH * 0.5f && normY <= boundsY + boundsH * 0.5f) {
            isDragging_ = true;
            draggedThresholdExceeded_ = false;
            startY_ = normY;
            startOffset_ = scrollOffset_;
            totalDragDistance_ = 0.0f;
        } else {
            isDragging_ = false;
        }
    }

    void onTouchMove(float normX, float normY) {
        if (!isDragging_) return;
        float dy = normY - startY_; // Delta in normalized UI coords [-1, 1]
        float deltaScroll = -dy;   // Moving finger UP (dy > 0) scrolls down (increases offset)
        totalDragDistance_ += std::abs(dy);
        if (totalDragDistance_ > 0.025f) {
            draggedThresholdExceeded_ = true;
        }
        scrollOffset_ = std::clamp(startOffset_ + deltaScroll, 0.0f, maxScrollOffset_);
    }

    void onTouchUp() {
        isDragging_ = false;
    }

    [[nodiscard]] bool wasDragged() const {
        return draggedThresholdExceeded_;
    }

    [[nodiscard]] float getScrollOffset() const {
        return scrollOffset_;
    }

    void resetScroll() {
        scrollOffset_ = 0.0f;
        isDragging_ = false;
        draggedThresholdExceeded_ = false;
    }

    void renderScrollbar(const Shader& shader, const float* ortho,
                         float trackX, float trackY, float trackW, float trackH) const {
        if (maxScrollOffset_ <= 0.001f || contentHeight_ <= 0.001f) return;

        // Draw scrollbar background track
        UIDrawHelpers::drawQuad(shader, ortho, trackX, trackY, trackW, trackH,
                                0.08f, 0.12f, 0.20f, 0.85f);

        // Calculate thumb height and vertical position
        float visibleRatio = std::min(1.0f, viewportHeight_ / contentHeight_);
        float thumbH = std::max(0.08f, trackH * visibleRatio);
        float scrollRatio = (maxScrollOffset_ > 0.0f) ? (scrollOffset_ / maxScrollOffset_) : 0.0f;

        float thumbMinY = trackY - (trackH * 0.5f) + (thumbH * 0.5f);
        float thumbMaxY = trackY + (trackH * 0.5f) - (thumbH * 0.5f);
        float thumbY = thumbMaxY - scrollRatio * (thumbMaxY - thumbMinY);

        // Draw scroll thumb handle
        UIDrawHelpers::drawQuad(shader, ortho, trackX, thumbY, trackW, thumbH,
                                0.20f, 0.70f, 0.95f, 0.95f);
    }

private:
    float scrollOffset_;
    float maxScrollOffset_;
    float viewportHeight_;
    float contentHeight_;
    bool isDragging_;
    bool draggedThresholdExceeded_;
    float startY_;
    float startOffset_;
    float totalDragDistance_;
};

#endif // TOUCHPARTY_UISCROLLCONTAINER_H
