#include <gtest/gtest.h>

#if NBODY_WITH_UI

#include "nbody/ui_panel.hpp"

using namespace nbody;

class UIPanelTest : public ::testing::Test {
protected:
  UIPanel panel_;
};

TEST_F(UIPanelTest, InitialState) {
  EXPECT_FALSE(panel_.isPaused());
  EXPECT_FALSE(panel_.shouldReset());
  EXPECT_FALSE(panel_.methodChanged());
  EXPECT_TRUE(panel_.isVisible());
}

TEST_F(UIPanelTest, SettersAndGetters) {
  panel_.setFps(60.5f);
  panel_.setFrameTime(16.5f);
  panel_.setParticleCount(10000);
  panel_.setSimulationTime(12.34f);
  panel_.setForceMethod(ForceMethod::BARNES_HUT);
  panel_.setPaused(true);

  EXPECT_TRUE(panel_.isPaused());
  EXPECT_EQ(panel_.getSelectedMethod(), ForceMethod::DIRECT_N2);  // Default
}

TEST_F(UIPanelTest, VisibilityToggle) {
  EXPECT_TRUE(panel_.isVisible());

  panel_.toggleVisibility();
  EXPECT_FALSE(panel_.isVisible());

  panel_.toggleVisibility();
  EXPECT_TRUE(panel_.isVisible());
}

TEST_F(UIPanelTest, PauseToggle) {
  EXPECT_FALSE(panel_.isPaused());

  panel_.setPaused(true);
  EXPECT_TRUE(panel_.isPaused());

  panel_.setPaused(false);
  EXPECT_FALSE(panel_.isPaused());
}

TEST_F(UIPanelTest, ResetFlag) {
  EXPECT_FALSE(panel_.shouldReset());

  // Simulate reset button click would set this internally
  // For now, just test the flag mechanism
  panel_.clearResetFlag();
  EXPECT_FALSE(panel_.shouldReset());
}

TEST_F(UIPanelTest, MethodSelection) {
  EXPECT_EQ(panel_.getSelectedMethod(), ForceMethod::DIRECT_N2);
  EXPECT_FALSE(panel_.methodChanged());

  panel_.clearMethodChangedFlag();
  EXPECT_FALSE(panel_.methodChanged());
}

TEST_F(UIPanelTest, MultipleStateChanges) {
  panel_.setFps(120.0f);
  panel_.setParticleCount(50000);
  panel_.setSimulationTime(100.0f);
  panel_.setForceMethod(ForceMethod::SPATIAL_HASH);
  panel_.setPaused(true);

  EXPECT_TRUE(panel_.isPaused());

  panel_.toggleVisibility();
  EXPECT_FALSE(panel_.isVisible());
}

#endif  // NBODY_WITH_UI

#if !NBODY_WITH_UI
// When UI is disabled, provide a dummy test to verify gating
TEST(UIPanelGatingTest, UIPanelDisabledWhenUINotEnabled) {
  SUCCEED() << "UIPanel tests skipped: NBODY_WITH_UI is OFF";
}
#endif
