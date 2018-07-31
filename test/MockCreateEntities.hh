/*
 * Copyright (C) 2018 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef IGNITION_PHYSICS_TEST_MOCKCREATEENTITY_HH_
#define IGNITION_PHYSICS_TEST_MOCKCREATEENTITY_HH_

#include <memory>
#include <string>

#include <ignition/math/Box.hh>
#include <ignition/math/Cylinder.hh>
#include <ignition/math/Inertial.hh>
#include <ignition/math/Pose3.hh>
#include <ignition/math/Sphere.hh>

#include <ignition/physics/FeatureList.hh>
#include <ignition/physics/FramedQuantity.hh>
#include <ignition/physics/SpecifyData.hh>

namespace mock
{
  struct Geometry
  {
    ignition::physics::ExpectData<
      ignition::math::Boxd,
      ignition::math::Cylinderd,
      ignition::math::Sphered
      > data;
  };

  struct RelativePose
  {
    ignition::math::Pose3d pose;
    std::string relative_to;
  };

  struct NamedWithRelativePose
  {
    std::string name;
    RelativePose pose;
  };

  struct Collision : NamedWithRelativePose
  {
    Geometry geometry;
  };

  enum MotionType
  {
    STATIC = 0,
    KINEMATIC,
    DYNAMIC
  };

  struct Link : NamedWithRelativePose
  {
    ignition::math::Inertiald inertial;
    std::vector<Collision> collisions;
    MotionType motion_type;
  };

  struct Joint : NamedWithRelativePose
  {
    std::string parent_link_name;
    std::string child_link_name;
  };

  struct ModelFreeLink : NamedWithRelativePose
  {
    Link link;
    MotionType motion_type;
  };

  struct Model : NamedWithRelativePose
  {
    std::vector<Joint> joints;
    std::vector<Link> links;
    std::vector<Model> models;
    MotionType motion_type;
  };

  /////////////////////////////////////////////////
  /// \brief A feature for creating and retrieving links from an engine. This
  /// is used by the mock Frame Semantics plugin so that we can test the
  /// Frame Semantics feature.
  struct MockCreateEntities : public ignition::physics::Feature
  {
    using Identity = ignition::physics::Identity;

    template <typename PolicyT, typename FeaturesT>
    class Engine : public virtual Feature::Engine<PolicyT, FeaturesT>
    {
      public: using FrameData =
        ignition::physics::FrameData<typename PolicyT::Scalar, PolicyT::Dim>;

      public: using Link = ignition::physics::Link<PolicyT, FeaturesT>;
      public: using Joint = ignition::physics::Joint<PolicyT, FeaturesT>;

      /// \brief Create a link, giving it a name and data. The data is relative
      /// to the world frame.
      public: std::unique_ptr<Link> CreateLink(
          const std::string &_linkName,
          const FrameData &_frameData);

      /// \brief Create a joint, giving it a name and data. The data is relative
      /// to the world frame.
      public: std::unique_ptr<Joint> CreateJoint(
          const std::string &_jointName,
          const FrameData &_frameData);

      /// \brief Retrieve a link that was created earlier.
      public: std::unique_ptr<Link> GetLink(const std::string &_linkName) const;

      /// \brief Retrieve a joint that was created earlier.
      public: std::unique_ptr<Joint> GetJoint(
          const std::string &_jointName) const;
    };

    template <typename PolicyT, typename FeaturesT>
    class World : public virtual Feature::World<PolicyT, FeaturesT>
    {
      public: using Model = ignition::physics::Model<PolicyT, FeaturesT>;

      /// \brief Create a link, giving it a name and data. The data is relative
      /// to the world frame.
      public: std::unique_ptr<Model> CreateModelFreeLink(
          const ModelFreeLink &_model);
    };

    template <typename PolicyT>
    class Implementation : public virtual Feature::Implementation<PolicyT>
    {
      public: using FrameData =
        ignition::physics::FrameData<typename PolicyT::Scalar, PolicyT::Dim>;

      public: virtual Identity CreateModelFreeLink(
          const ModelFreeLink &_model) = 0;

      public: virtual Identity CreateModelFreeLink(
          const ModelFreeLink &_model) const = 0;

      public: virtual Identity CreateLink(
          const std::string &_linkName,
          const FrameData &_frameData) = 0;

      public: virtual Identity CreateJoint(
          const std::string &_jointName,
          const FrameData &_frameData) = 0;

      public: virtual Identity GetLink(
          const std::string &_linkName) const = 0;

      public: virtual Identity GetJoint(
          const std::string &_jointName) const = 0;
    };
  };


  // ---------------------- Implementations ----------------------

  /////////////////////////////////////////////////
  template <typename PolicyT, typename FeaturesT>
  auto MockCreateEntities::Engine<PolicyT, FeaturesT>::CreateLink(
      const std::string &_linkName,
      const FrameData &_frameData) -> std::unique_ptr<Link>
  {
    const Identity linkId =
        this->template Interface<MockCreateEntities>()
          ->CreateLink(_linkName, _frameData);

    if (!linkId)
      return nullptr;

    return std::make_unique<Link>(this->pimpl, linkId);
  }

  /////////////////////////////////////////////////
  template <typename PolicyT, typename FeaturesT>
  auto MockCreateEntities::Engine<PolicyT, FeaturesT>::CreateJoint(
      const std::string &_jointName,
      const FrameData &_frameData) -> std::unique_ptr<Joint>
  {
    const Identity jointId =
        this->template Interface<MockCreateEntities>()
          ->CreateJoint(_jointName, _frameData);

    if (!jointId)
      return nullptr;

    return std::make_unique<Joint>(this->pimpl, jointId);
  }

  /////////////////////////////////////////////////
  template <typename PolicyT, typename FeaturesT>
  auto MockCreateEntities::Engine<PolicyT, FeaturesT>::GetLink(
      const std::string &_linkName) const -> std::unique_ptr<Link>
  {
    const Identity linkId =
        this->template Interface<MockCreateEntities>()->GetLink(_linkName);

    if (!linkId)
      return nullptr;

    return std::make_unique<Link>(this->pimpl, linkId);
  }

  /////////////////////////////////////////////////
  template <typename PolicyT, typename FeaturesT>
  auto MockCreateEntities::Engine<PolicyT, FeaturesT>::GetJoint(
      const std::string &_jointName) const -> std::unique_ptr<Joint>
  {
    const Identity jointId =
        this->template Interface<MockCreateEntities>()->GetJoint(_jointName);

    if (!jointId)
      return nullptr;

    return std::make_unique<Joint>(this->pimpl, jointId);
  }
}

#endif
