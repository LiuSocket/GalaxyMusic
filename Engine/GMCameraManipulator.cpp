//////////////////////////////////////////////////////////////////////////
/// COPYRIGHT NOTICE
/// Copyright (c) 2020~2030, LiuTao
/// All rights reserved.
///
/// @file		GMCameraManipulator.cpp
/// @brief		Galaxy-Music Engine - GMCameraManipulator
/// @version	1.0
/// @author		LiuTao
/// @date		2021.05.30
//////////////////////////////////////////////////////////////////////////
#include "GMCameraManipulator.h"
#include "GMEngine.h"
#include "GMSystemManager.h"

//#include <iostream>

using namespace GM;

/*************************************************************************
 Macro Defines
*************************************************************************/

#define GM_MAX_DIR_Z				(0.999)
#define GM_HOME_PITCH				(-osg::PI*0.11)
#define GM_GOTO_PITCH				(-osg::PI*0.04)
#define GM_HOME_RADIUS				(9.0)
#define GM_HOME_DURATION			(8.0)
#define GM_GOTO_DURATION			(6.0)
#define GM_MAX_DISTANCE				(GM_MAX_RADIUS*2)
#define GM_NEARFAR_RATIO			(1e-5)
#define GM_CURSOR_TIME				(3.0f)
#define GM_ULTIMATE_RADIUS			(0.0017474774194546)

/*************************************************************************
CGMCameraManipulator Methods
*************************************************************************/

CGMCameraManipulator::CGMCameraManipulator()
	: osgGA::CameraManipulator(), m_vTranslate(osg::Vec3d(0, -9, 9)), 
	m_qRotate(osg::Quat(-osg::PI_4, osg::Vec3d(1, 0, 0), 0, osg::Vec3d(0, 1, 0), 0, osg::Vec3d(0, 0, 1))),
	m_fDeltaStep(0.0f), m_fConstantStep(0.5f), m_fCursorStillTime(0.0f),
	m_fTimeLastFrame(0.0), m_fTimeJumpStart(0.0), m_fTimeSinceJump(0.0), m_fJumpDuration(GM_GOTO_DURATION),
	m_fCameraRadius(GM_HOME_RADIUS), m_fCameraStartRadius(GM_HOME_RADIUS), m_fRadiusAcceleration(0.0), m_fRadiusWeakRatio(3),
	m_fConstSpinSpeed(osg::inDegrees(0.6)), m_fSpinSpeed(osg::inDegrees(0.0)),m_fPitchSpeed(0.0),
	m_fTerminalPitch(GM_HOME_PITCH), m_fTerminalYaw(0.0), m_fOriginPitch(GM_HOME_PITCH), m_fOriginYaw(0.0),
	m_fSpinAcceleration(0.0), m_fSpinWeakRatio(3),
	m_fMixA(1.3*GM_GOTO_DURATION), m_fMixB(2.6*GM_GOTO_DURATION), m_fMixMax(0.99),
	m_vTargetHierarchyPos(osg::Vec3d(0, 0, 0)),
	m_vTerminalHierarchyPos(osg::Vec3d(0, 0, 0)), m_vOriginHierarchyPos(osg::Vec3d(0, 0, 0)),
	m_vMouseHierarchyPos(osg::Vec3d(0, 0, 0)), m_vLastMouseHierarchyPos(osg::Vec3d(0, 0, 0)),
	m_vStarWorldPos(osg::Vec3d(0, 0, 0)),
	m_vMousePushScreenPos(osg::Vec2f(0.0f, 0.0f)), m_vMouseLastScreenPos(osg::Vec2f(0.0f,0.0f)),
	m_vMouseDragLastScreenPos(osg::Vec2f(0.0f, 0.0f)),
	m_eFlyMode(EGMA_FLY_FREE), m_bEdit(false), m_bDrag(false)
{
	osg::Vec3d vDirection = osg::Vec3d(0.0, std::cos(m_fOriginPitch), std::sin(m_fOriginPitch));
	m_vTranslate = _GetTranslate(vDirection);

	osg::Matrix mCameraRotate;
	mCameraRotate.makeLookAt(osg::Vec3d(0, 0, 0), vDirection, osg::Vec3d(0, 0, 1));
	m_qRotate.set(mCameraRotate);
}

bool
CGMCameraManipulator::screen2Hierarchy(float x, float y, osg::View* theView, osg::Vec3d& vHierarchy) const
{
	osgViewer::View* view = dynamic_cast<osgViewer::View*>(theView);
	if (!view) return false;

	const osg::Camera* camera = view->getCamera();
	if (!camera) return false;

	osg::Matrix matrix = camera->getViewMatrix();
	matrix.postMult(camera->getProjectionMatrix());

	double zNear = -1.0;
	double zFar = 1.0;
	if (camera->getViewport())
	{
		matrix.postMult(camera->getViewport()->computeWindowMatrix());
		zNear = 0.0;
		zFar = 1.0;
	}

	osg::Matrixd inverse;
	inverse.invert(matrix);

	osg::Vec3d startVertex = osg::Vec3d(x, y, zNear) * inverse;
	osg::Vec3d endVertex = osg::Vec3d(x, y, zFar) * inverse;

	//不与XY平面相交，无法计算相交点
	if (startVertex.z()*endVertex.z() >= 0)
		return false;

	double fRatio = abs(startVertex.z() / max(1e-20,abs(endVertex.z() - startVertex.z())));
	vHierarchy = osg::Vec3d(
		startVertex.x() + fRatio * (endVertex.x() - startVertex.x()),
		startVertex.y() + fRatio * (endVertex.y() - startVertex.y()),
		0);

	return true;
}

double CGMCameraManipulator::GetHierarchyTargetDistance()
{
	return m_fCameraRadius;
}

void
CGMCameraManipulator::setByMatrix(const osg::Matrixd& matrix)
{
	m_vTranslate = matrix.getTrans();
	m_qRotate = matrix.getRotate();
}

osg::Matrixd
CGMCameraManipulator::getMatrix() const
{
	osg::Matrix mat;
	mat.setRotate(m_qRotate);//先旋转
	mat.postMultTranslate(m_vTranslate);//后平移
	return mat;
}

osg::Matrixd
CGMCameraManipulator::getInverseMatrix() const
{
	osg::Matrix mat;
	mat.setRotate(-m_qRotate);//后旋转
	mat.preMultTranslate(-m_vTranslate);//先平移
	return mat;
}

bool
CGMCameraManipulator::handle(const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa)
{
	switch (ea.getEventType())
	{
	case (osgGA::GUIEventAdapter::FRAME):
	{	
		double timeCurrFrame = osg::Timer::instance()->time_s();
		double fDeltaTime = timeCurrFrame - m_fTimeLastFrame;

		float fInnerDeltaTime = fDeltaTime;
		fInnerDeltaTime += m_fDeltaStep;
		float updateStep = m_fConstantStep;
		while (fInnerDeltaTime >= updateStep)
		{
			_InnerUpdate(updateStep);
			fInnerDeltaTime -= updateStep;
		}
		m_fDeltaStep = fInnerDeltaTime;

		int iHie = GM_ENGINE_PTR->GetHierarchy();
		osg::Vec3d vDirection = osg::Vec3d(0, 1, 0);
		switch (m_eFlyMode)
		{
		case EGMA_FLY_FREE:
		case EGMA_FLY_CONTROL:
		{
			// 目标点混合系数
			double fTargetMix = min(1.0, 0.5*fDeltaTime);

			if (EGMA_FLY_CONTROL == m_eFlyMode)
			{
				float fDeltaY = ea.getY() - m_vMousePushScreenPos.y();
				float fSignY = osg::sign(fDeltaY);
				m_fRadiusAcceleration = m_fCameraRadius * fSignY * fDeltaTime * min(4.0, abs(fDeltaY) * 0.004);
				// 当远离目标时，目标点锁定
				if(fDeltaY >= 0.0) fTargetMix = 0.0;
			}

			// 修改姿态
			double fTargetYaw = _Mix(m_fOriginYaw, m_fTerminalYaw, fTargetMix);
			double fTargetPitch = _Mix(m_fOriginPitch, m_fTerminalPitch, fTargetMix);
			double fDeltaYaw = fTargetYaw - m_fOriginYaw;
			double fDeltaPitch = fTargetPitch - m_fOriginPitch;
			m_fOriginYaw = fTargetYaw;
			m_fOriginPitch = fTargetPitch;

			m_fSpinSpeed = osg::clampBetween(m_fSpinSpeed + m_fSpinAcceleration, -6.0, 6.0);
			m_fSpinSpeed *= exp2(-fDeltaTime * m_fSpinWeakRatio);

			// 修改姿态
			osg::Matrix mCameraRotate;
			m_qRotate.get(mCameraRotate);
			osg::Vec3d vEye, vCenter, vUp; //相机的相对姿态
			mCameraRotate.getLookAt(vEye, vCenter, vUp);//这里的vEye不是实际相机的位置，而是0，0，0
			vDirection = vCenter - vEye; //相机视点方向
			osg::Vec3d vRight = vDirection ^ vUp;
			vRight.normalize();

			// 判断俯仰角是否超过界限
			if ((vDirection.z() < GM_MAX_DIR_Z) && (m_fPitchSpeed >= 0)
				|| (vDirection.z() > -GM_MAX_DIR_Z) && (m_fPitchSpeed <= 0))
			{
				osg::Matrix mDeltaRotateHorizontal = osg::Matrix::rotate(
					(m_fSpinSpeed + m_fConstSpinSpeed) * fDeltaTime - fDeltaYaw,
					osg::Vec3d(0, 0, 1));
				osg::Matrix mDeltaRotateVertical = osg::Matrix::rotate(m_fPitchSpeed * fDeltaTime + fDeltaPitch, vRight);
				vDirection = vDirection * mDeltaRotateHorizontal * mDeltaRotateVertical;
				vDirection.normalize();
			}
			mCameraRotate.makeLookAt(osg::Vec3d(0, 0, 0), vDirection, osg::Vec3d(0, 0, 1));
			m_qRotate.set(mCameraRotate);

			m_vTargetHierarchyPos = m_vTargetHierarchyPos * (1 - fTargetMix) + m_vTerminalHierarchyPos * fTargetMix;
			m_fCameraRadius = osg::clampBetween(m_fCameraRadius + m_fRadiusAcceleration, GM_MIN_RADIUS, GM_MAX_DISTANCE);
			m_fRadiusAcceleration *= exp2(-fDeltaTime * m_fRadiusWeakRatio);
		}
		break;
		case EGMA_FLY_GOTO:
		case EGMA_FLY_HOME:
		case EGMA_FLY_JUMP:
		{
			double fMix = 0.0; // 半径、角度的混合系数
			m_fTimeSinceJump = timeCurrFrame - m_fTimeJumpStart; //单位:秒
			double fT = m_fTimeSinceJump / m_fJumpDuration;
			double C = m_fMixA + m_fMixB;
			fMix = 1 + (m_fMixB / m_fMixA)*exp(-fT * C) - (C / m_fMixA)*exp(-fT * m_fMixB);
			fMix = min(1.0, fMix / m_fMixMax);
			// 目标点混合系数
			double fTargetMix = _Smoothstep(0, 1, fMix);

			if (4 == iHie)
			{
				if (EGMA_FLY_GOTO == m_eFlyMode)
				{
					if (m_fTimeSinceJump > m_fJumpDuration || m_fCameraRadius <= GM_MIN_RADIUS)
					{
						_SetFlyMode(EGMA_FLY_FREE);
					}
					m_fCameraRadius = _Mix(m_fCameraStartRadius, GM_MIN_RADIUS, fMix);
				}
				else
				{
					if (m_fTimeSinceJump > m_fJumpDuration)
					{
						if (EGMA_FLY_JUMP == m_eFlyMode)
						{
							_SetFlyMode(EGMA_FLY_GOTO);
							fMix = 0.0;
							fTargetMix = 0.0;
						}
						else if (EGMA_FLY_HOME == m_eFlyMode)
						{
							_SetFlyMode(EGMA_FLY_FREE);
							fMix = 0.0;
							fTargetMix = 0.0;
						}
					}
					m_fCameraRadius = _Mix(m_fCameraStartRadius, GM_HOME_RADIUS, fMix);
				}
			}
			else if (iHie < 4)
			{
				if (EGMA_FLY_GOTO == m_eFlyMode)
				{
					if (m_fTimeSinceJump > m_fJumpDuration || m_fCameraRadius <= GM_MIN_RADIUS)
					{
						_SetFlyMode(EGMA_FLY_FREE);
					}
					m_fCameraRadius = _Mix(m_fCameraStartRadius, GM_MIN_RADIUS, fMix);
				}
				else
				{
					m_fCameraRadius = _Mix(m_fCameraStartRadius, GM_HOME_RADIUS*std::pow(GM_UNIT_SCALE,4-iHie), fMix);
				}
			}

			// 目标点混合
			m_vTargetHierarchyPos = m_vOriginHierarchyPos * (1 - fTargetMix) + m_vTerminalHierarchyPos * fTargetMix;

			// 修改姿态
			double fTargetYaw = _Mix(m_fOriginYaw, m_fTerminalYaw, fMix);
			double fTargetPitch = _Mix(m_fOriginPitch, m_fTerminalPitch, fMix);

			double fZ = std::sin(fTargetPitch);
			double fXY = std::sqrt(1 - fZ * fZ);
			vDirection.z() = fZ;
			vDirection.x() = fXY * std::sin(fTargetYaw);
			vDirection.y() = fXY * std::cos(fTargetYaw);

			osg::Matrix mCameraRotate;
			mCameraRotate.makeLookAt(osg::Vec3d(0, 0, 0), vDirection, osg::Vec3d(0, 0, 1));
			m_qRotate.set(mCameraRotate);
		}
		break;
		case EGMA_FLY_ULTIMATE:
		{
			if (6 == iHie)
			{
				if (m_fCameraRadius >= GM_ULTIMATE_RADIUS)
				{
					_SetFlyMode(EGMA_FLY_FREE);
				}
				else
				{
					m_fCameraRadius = 0.98*m_fCameraRadius + 0.02*GM_ULTIMATE_RADIUS;
				}
			}
			else
			{
				m_fCameraRadius *= 1.01;
			}

			double fMix = 0.0; // 半径、角度的混合系数
			m_fTimeSinceJump = timeCurrFrame - m_fTimeJumpStart; //单位:秒
			double fT = m_fTimeSinceJump / m_fJumpDuration;
			double C = m_fMixA + m_fMixB;
			fMix = 1 + (m_fMixB / m_fMixA)*exp(-fT * C) - (C / m_fMixA)*exp(-fT * m_fMixB);
			fMix = min(1.0, fMix / m_fMixMax);
			// 目标点混合系数
			double fTargetMix = _Smoothstep(0, 1, fMix);

			// 修改姿态
			double fTargetYaw = _Mix(m_fOriginYaw, m_fTerminalYaw, fMix);
			double fTargetPitch = _Mix(m_fOriginPitch, m_fTerminalPitch, fMix);

			double fZ = std::sin(fTargetPitch);
			double fXY = std::sqrt(1 - fZ * fZ);
			vDirection.z() = fZ;
			vDirection.x() = fXY * std::sin(fTargetYaw);
			vDirection.y() = fXY * std::cos(fTargetYaw);

			osg::Matrix mCameraRotate;
			mCameraRotate.makeLookAt(osg::Vec3d(0, 0, 0), vDirection, osg::Vec3d(0, 0, 1));
			m_qRotate.set(mCameraRotate);
		}
		break;
		}

		// 更新 m_fCursorStillTime
		if (m_vMouseLastScreenPos == osg::Vec2f(ea.getX(), ea.getY()))
		{
			m_fCursorStillTime += fDeltaTime;

		}
		else if(m_fCursorStillTime > GM_CURSOR_TIME)
		{
			m_fCursorStillTime = -0.001f;
		}
		//// 显示或隐藏鼠标,但三维窗口不成功，暂时关闭这个功能
		//if ((m_fCursorStillTime - fDeltaTime) < GM_CURSOR_TIME && m_fCursorStillTime > GM_CURSOR_TIME)
		//{
		//	GM_SYSTEM_MANAGER.SetCursorVisible(false);
		//}
		//else if (m_fCursorStillTime < 0.0f)
		//{
		//	GM_SYSTEM_MANAGER.SetCursorVisible(true);
		//}

		if (screen2Hierarchy(ea.getX(), ea.getY(), aa.asView(), m_vMouseHierarchyPos))
		{
			GM_ENGINE_PTR->SetMousePosition(m_vMouseHierarchyPos);
		}

		// 获取跳跃前的空间坐标
		m_vTranslate = _GetTranslate(vDirection);
		// 空间层级切换
		// 注意：空间层级每隔5个数量级就切换一次
		// 但上切换和下切换的尺度相差一半，最大半径和最小半径的比值刻意设置成2e5
		// 上述比值是为了避免来回切换，不要修改它！！！！
		// 当相机半径不大于最小半径（5e-4,本空间下）时，则切换到下一层级空间
		// 当相机半径不小大于最大半径（1e2,本空间下）时，则切换到上一层级空间
		// 半径 <= 5e21，5跳4	半径 >= 1e22，4跳5
		// 半径 <= 5e16，4跳3	半径 >= 1e17，3跳4
		// 半径 <= 5e11，3跳2	半径 >= 1e12，2跳3
		// 半径 <= 5e6，2跳1	半径 >= 1e7，1跳2
		if (m_fCameraRadius <= GM_MIN_RADIUS)
		{
			if (GM_ENGINE_PTR->SubHierarchy(m_vTranslate, m_vTargetHierarchyPos))
			{
				m_vTranslate = GM_ENGINE_PTR->GetHierarchyLastEyePos();
				if (4 <= GM_ENGINE_PTR->GetHierarchy())
				{
					m_vTerminalHierarchyPos = m_vStarWorldPos / GM_ENGINE_PTR->GetUnit();
				}
				else
				{
					m_vTerminalHierarchyPos = (m_vTerminalHierarchyPos - m_vTargetHierarchyPos)*GM_UNIT_SCALE;
				}
				m_vOriginHierarchyPos = (m_vOriginHierarchyPos - m_vTargetHierarchyPos)*GM_UNIT_SCALE;
				m_vTargetHierarchyPos = GM_ENGINE_PTR->GetHierarchyLastTargetPos();
				m_fCameraRadius *= GM_UNIT_SCALE;
				m_fCameraStartRadius *= GM_UNIT_SCALE;
			}
		}
		else if (m_fCameraRadius >= GM_MAX_RADIUS && GM_HIERARCHY_MAX > GM_ENGINE_PTR->GetHierarchy())
		{
			if (GM_ENGINE_PTR->AddHierarchy(m_vTranslate, m_vTargetHierarchyPos))
			{
				m_vTargetHierarchyPos = GM_ENGINE_PTR->GetHierarchyLastTargetPos();
				m_vTranslate = GM_ENGINE_PTR->GetHierarchyLastEyePos();
				if ((EGMA_FLY_HOME == m_eFlyMode || EGMA_FLY_JUMP == m_eFlyMode)
					&& 4 == GM_ENGINE_PTR->GetHierarchy())
				{
					m_vTerminalHierarchyPos = osg::Vec3d(0,0,0);
				}
				else
				{
					m_vTerminalHierarchyPos = m_vTerminalHierarchyPos / GM_UNIT_SCALE + m_vTargetHierarchyPos;
				}
				m_vOriginHierarchyPos = m_vOriginHierarchyPos / GM_UNIT_SCALE + m_vTargetHierarchyPos;
				m_fCameraRadius /= GM_UNIT_SCALE;
				m_fCameraStartRadius /= GM_UNIT_SCALE;
			}
		}

		osg::Camera* camera = aa.asView()->getCamera();
		if (camera)
		{
			double fFovy, fAspectRatio, fNear, fFar;
			camera->getProjectionMatrixAsPerspective(fFovy, fAspectRatio, fNear, fFar);
			double fDistance = m_vTranslate.length();

			double fFarNew = 10.0;
			double fNearNew = fFarNew * GM_NEARFAR_RATIO;
			switch (iHie)
			{
			case 2:
			case 3:
			{
				fNearNew = osg::clampBetween(fDistance - 1e-2, 1e-4, 0.4);
				fFarNew = fNearNew / GM_NEARFAR_RATIO;
			}
			break;
			case 4:
			{

				if (fDistance < GM_ENGINE_PTR->GetGalaxyRadius())// 在星系内部
				{
					fFarNew = osg::clampBetween(
						GM_ENGINE_PTR->GetGalaxyRadius() * 2 / GM_ENGINE_PTR->GetUnit()
						+ fDistance, 2.0, 1000.0);
					fNearNew = fFarNew * GM_NEARFAR_RATIO;
				}
				else// 在星系外面
				{
					fNearNew = osg::clampBetween(
						fDistance - GM_ENGINE_PTR->GetGalaxyRadius() * 2 / GM_ENGINE_PTR->GetUnit(),
						2e-5, 0.4);
					fFarNew = fNearNew / GM_NEARFAR_RATIO;
				}
			}
			break;
			case 5:
			{
				fNearNew = osg::clampBetween(
					fDistance - GM_ENGINE_PTR->GetGalaxyRadius() / GM_ENGINE_PTR->GetUnit(),
					1e-4, _Mix(2e-3, 0.05, osg::clampBetween(fDistance/50.0, 0.0, 1.0)));
				fFarNew = fNearNew / GM_NEARFAR_RATIO;
			}
			break;
			default:
				break;
			}
			camera->setProjectionMatrixAsPerspective(fFovy, fAspectRatio, fNearNew, fFarNew);
		}

		m_fPitchSpeed *= exp2(-fDeltaTime * 2.0);
		m_fSpinAcceleration *= exp2(-fDeltaTime * 7.5);
		m_fTimeLastFrame = timeCurrFrame;

		m_vMouseLastScreenPos = osg::Vec2f(ea.getX(), ea.getY());
	}
	break;
	case (osgGA::GUIEventAdapter::PUSH):
	{
		m_vLastMouseHierarchyPos = m_vMouseHierarchyPos;
		m_vMousePushScreenPos = osg::Vec2f(ea.getX(), ea.getY());
		m_vMouseDragLastScreenPos = m_vMousePushScreenPos;

		if (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON == ea.getButton())
		{
		}
		else if (osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON == ea.getButton())
		{
			if (!m_bEdit)
			{
				m_fRadiusWeakRatio = 20;
				m_fSpinWeakRatio = 20;
			}
		}
	}
	break;
	case (osgGA::GUIEventAdapter::RELEASE):
	{
		if (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON == ea.getButton())
		{
			m_bDrag = false;
			_SetFlyMode(EGMA_FLY_FREE);
			GM_ENGINE_PTR->SetCapture(false);
		}
		else if (osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON == ea.getButton())
		{
			m_fRadiusWeakRatio = 3;
			m_fSpinWeakRatio = 3;
		}
	}
	break;
	case (osgGA::GUIEventAdapter::DRAG):
	{
		if (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON == ea.getButtonMask())
		{
			if (!m_bDrag)
			{
				_SetFlyMode(EGMA_FLY_CONTROL);
				m_bDrag = true;
			}

			if (m_bEdit)
			{
				GM_ENGINE_PTR->SetCapture(true);
			}
		}
		else if (osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON == ea.getButtonMask())
		{
			float fDeltaX = ea.getX() - m_vMouseDragLastScreenPos.x();
			float fDeltaY = ea.getY() - m_vMouseDragLastScreenPos.y();
			if (fDeltaX == 0.0f || 1.0f < std::abs(fDeltaY / fDeltaX)) // 控制俯仰
			{
				m_fPitchSpeed = (m_fPitchSpeed + fDeltaY * 0.09) * 0.5;
			}
			else // 控制自传
			{
				int iHie = GM_ENGINE_PTR->GetHierarchy();
				if (4 == iHie)
				{
					double fAngular = _GetAngularVelocity(m_vMouseHierarchyPos, m_vLastMouseHierarchyPos);
					m_fSpinAcceleration = 3 * fAngular;
				}
				else
				{
					m_fSpinAcceleration = 0.01*(ea.getX()-m_vMouseLastScreenPos.x())*osg::sign(ea.getYnormalized());
				}
			}
		}

		m_vLastMouseHierarchyPos = m_vMouseHierarchyPos;
		m_vMouseDragLastScreenPos = osg::Vec2f(ea.getX(), ea.getY());
	}
	break;
	case (osgGA::GUIEventAdapter::SCROLL):
	{
	}
	break;
	case (osgGA::GUIEventAdapter::KEYDOWN):
	{
		switch (ea.getKey())
		{
		case osgGA::GUIEventAdapter::KEY_Left:
		{
			m_fSpinAcceleration = - m_fSpinSpeed * 0.07;
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Right:
		{
			m_fSpinAcceleration = 0.06;
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Up:
		{
			m_fPitchSpeed = -0.06;
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Down:
		{
			m_fPitchSpeed = 0.06;
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Space:
		{
			_SetFlyMode(EGMA_FLY_HOME);
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Return:
		{
			_SetFlyMode(EGMA_FLY_GOTO);
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F2:
		{
			GM_ENGINE_PTR->SetPlayMode(EGMA_MOD_CIRCLE);
			GM_ENGINE_PTR->Next();

			int iHie = GM_ENGINE_PTR->GetHierarchy();
			if (4 == iHie)
			{
				if (m_fCameraRadius >= GM_HOME_RADIUS)
				{
					_SetFlyMode(EGMA_FLY_GOTO);
				}
				else
				{
					_SetFlyMode(EGMA_FLY_JUMP);
				}
			}
			else if (iHie < 4)
			{
				_SetFlyMode(EGMA_FLY_JUMP);
			}
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F3:
		{
			GM_ENGINE_PTR->SetPlayMode(EGMA_MOD_RANDOM);
			GM_ENGINE_PTR->Next();

			int iHie = GM_ENGINE_PTR->GetHierarchy();
			if (4 == iHie)
			{
				if (m_fCameraRadius >= GM_HOME_RADIUS)
				{
					_SetFlyMode(EGMA_FLY_GOTO);
				}
				else
				{
					_SetFlyMode(EGMA_FLY_JUMP);
				}
			}
			else if (iHie < 4)
			{
				_SetFlyMode(EGMA_FLY_JUMP);
			}
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F4:
		{
			m_bEdit = !m_bEdit;
			GM_ENGINE_PTR->SetEditMode(m_bEdit);
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F5:
		{
			GM_ENGINE_PTR->Load();
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F9:
		{
			_SetFlyMode(EGMA_FLY_ULTIMATE);
		}
		break;
		break;
		//case osgGA::GUIEventAdapter::KEY_5:
		//{
		//	// 太阳位置
		//	GM_ENGINE_PTR->SetAudio(0.0, -2.5e20);

		//	int iHie = GM_ENGINE_PTR->GetHierarchy();
		//	if (4 == iHie)
		//	{
		//		if (m_fCameraRadius >= GM_HOME_RADIUS)
		//		{
		//			_SetFlyMode(EGMA_FLY_GOTO);
		//		}
		//		else
		//		{
		//			_SetFlyMode(EGMA_FLY_JUMP);
		//		}
		//	}
		//	else if (iHie < 4)
		//	{
		//		_SetFlyMode(EGMA_FLY_JUMP);
		//	}
		//}
		//break;
		case 19:
		{
			// ctrl + s
			GM_ENGINE_PTR->Save();
		}
		break;
		}

		GM_SYSTEM_MANAGER.GMKeyDown(EGMKeyCode(ea.getKey()));
	}
	break;
	case (osgGA::GUIEventAdapter::KEYUP):
	{
		switch (ea.getKey())
		{
		case osgGA::GUIEventAdapter::KEY_Up:
		case osgGA::GUIEventAdapter::KEY_Down:
		{
			//m_fPitchSpeed = 0;
		}
		break;
		}

		GM_SYSTEM_MANAGER.GMKeyUp(EGMKeyCode(ea.getKey()));
	}
	break;
	case (osgGA::GUIEventAdapter::DOUBLECLICK):
	{
		if (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON == ea.getButtonMask())
		{
			if (4 == GM_ENGINE_PTR->GetHierarchy())
			{
				osg::Vec3d vMouseWorldPos = GM_ENGINE_PTR->Hierarchy2World(m_vMouseHierarchyPos);
				GM_ENGINE_PTR->SetAudio(vMouseWorldPos.x(), vMouseWorldPos.y());
			}
		}
		else if (osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON == ea.getButtonMask())
		{

		}
	}
	break;
	default:
		break;
	}

	return false;
}

CGMCameraManipulator::~CGMCameraManipulator()
{
}

void CGMCameraManipulator::_InnerUpdate(float updateStep)
{
	osg::Vec3d vNewStarWorldPos = GM_ENGINE_PTR->GetCurrentStarWorldPos();
	if (m_vStarWorldPos != vNewStarWorldPos)
	{
		m_vStarWorldPos = vNewStarWorldPos;
		if (EGMA_FLY_FREE == m_eFlyMode)
		{
			int iHie = GM_ENGINE_PTR->GetHierarchy();
			if (4 == iHie)
			{
				if (m_fCameraRadius >= GM_HOME_RADIUS)
				{
					_SetFlyMode(EGMA_FLY_GOTO);
				}
				else
				{
					_SetFlyMode(EGMA_FLY_JUMP);
				}
			}
			else if (iHie < 4)
			{
				_SetFlyMode(EGMA_FLY_JUMP);
			}
		}
	}
}

void
CGMCameraManipulator::_SetFlyMode(EGMC_FLY eMode)
{
	m_eFlyMode = eMode;

	osg::Vec3d vPos = osg::Vec3d(0, 0, 0);
	osg::Matrix mCameraRotate;
	m_qRotate.get(mCameraRotate);
	osg::Vec3d vEye, vCenter, vUp; //相机的相对姿态
	mCameraRotate.getLookAt(vEye, vCenter, vUp);//这里的vEye不是实际相机的位置，而是0，0，0
	osg::Vec3d vDirection = vCenter - vEye; //相机视点方向
	vDirection.normalize();
	m_fOriginPitch = asin(vDirection.z());
	m_fOriginYaw = atan2(vDirection.x(), vDirection.y());
	m_fCameraStartRadius = m_fCameraRadius;

	// 在“EGMA_FLY_HOME”过程中，相机yaw不变，pitch、目标点位置、焦距 随指数函数变化
	// 在“EGMA_FLY_GOTO”过程中，相机yaw，pitch、目标点位置、焦距 随指数函数变化

	switch (eMode)
	{
	case (EGMA_FLY_FREE):
	{
		if (m_fRadiusAcceleration >= 0.0)
		{
			vPos = m_vTargetHierarchyPos;
		}
		else
		{
			vPos = m_vTerminalHierarchyPos;
		}
	}
	break;
	case (EGMA_FLY_CONTROL):
	{
		int iHie = GM_ENGINE_PTR->GetHierarchy();
		if (4 == iHie)
		{
			vPos = m_vMouseHierarchyPos;
		}
		else
		{
			vPos = m_vTargetHierarchyPos;
		}
		osg::Vec3d vGo = vPos - m_vTranslate;

		m_fTerminalPitch = m_fOriginPitch;
		m_fTerminalYaw = atan2(vGo.x(), vGo.y());
	}
	break;
	case (EGMA_FLY_GOTO):
	{
		/* 由于本产品的特殊性，需要将银河系中心作为宇宙的中心
		* 这样就不用考虑用double表示恒星在6级空间中的位置的误差问题
		* 规定：恒星的坐标的最小单位长度 GM_STAR_POS_MIN
		* 也就是说，每颗音频星的三维世界坐标都是 GM_STAR_POS_MIN 的整数倍 */
		m_vStarWorldPos = GM_ENGINE_PTR->GetCurrentStarWorldPos();
		vPos = GM_ENGINE_PTR->StarWorld2Hierarchy(m_vStarWorldPos);

		osg::Vec3d vGo = vPos - m_vTranslate;
		double fDistance = vGo.length();

		m_fSpinSpeed = 0;
		m_fTerminalPitch = GM_GOTO_PITCH;
		m_fTerminalYaw = atan2(vGo.x(), vGo.y());

		m_fJumpDuration = GM_GOTO_DURATION * std::fmax(0.1, 1.0 - exp(-fDistance * 0.5 / GM_HOME_RADIUS));
		m_fMixA = 2.4*m_fJumpDuration;
		m_fMixB = 4.8*m_fJumpDuration;
	}
	break;
	case (EGMA_FLY_JUMP):
	case (EGMA_FLY_HOME):
	{
		m_fSpinSpeed = 0;
		m_fTerminalPitch = GM_HOME_PITCH;
		m_fTerminalYaw = m_fOriginYaw;

		int iHie = GM_ENGINE_PTR->GetHierarchy();
		if (4 == iHie)
		{
			osg::Vec3d vHomeEyePos = -vDirection * GM_HOME_RADIUS;
			double fDistance = (m_vTranslate - vHomeEyePos).length();
			m_fJumpDuration = GM_HOME_DURATION * (1 - exp(-fDistance * 0.5 / GM_HOME_RADIUS));
			m_fMixA = 1.8e-2*m_fJumpDuration;
			m_fMixB = 3.6e-2*m_fJumpDuration;
		}
		else if (iHie < 4)
		{
			m_fJumpDuration = GM_HOME_DURATION;
			m_fMixA = 1.8e-6*m_fJumpDuration;
			m_fMixB = 3.6e-6*m_fJumpDuration;
		}
	}
	break;
	case (EGMA_FLY_ULTIMATE):
	{
		m_fJumpDuration = 30.0;
		m_fMixA = 1e-6*m_fJumpDuration;
		m_fMixB = 2e-6*m_fJumpDuration;
		m_fSpinSpeed = 0;
		m_fTerminalPitch = GM_HOME_PITCH;
		m_fTerminalYaw = m_fOriginYaw;

		vPos = m_vTargetHierarchyPos;
	}
	break;
	}

	double C = m_fMixA + m_fMixB;
	m_fMixMax = 1 + (m_fMixB / m_fMixA)*exp(-C) - (C / m_fMixA)*exp(-m_fMixB);
	m_fTimeJumpStart = osg::Timer::instance()->time_s();
	m_fTimeSinceJump = 0.0;
	m_vOriginHierarchyPos = m_vTargetHierarchyPos;
	m_vTerminalHierarchyPos = vPos;
}

osg::Vec3d
CGMCameraManipulator::_GetTranslate(const osg::Vec3d vDirection)
{
	return m_vTargetHierarchyPos - vDirection * m_fCameraRadius;
}

double
CGMCameraManipulator::_GetAngularVelocity(const osg::Vec3d vHierarchyPos, const osg::Vec3d vLastHierarchyPos)
{
	osg::Vec3d vSpeed = vHierarchyPos - vLastHierarchyPos;
	osg::Vec3d vHierarchyPickDir = vHierarchyPos - m_vTargetHierarchyPos;
	double fPickLength = vHierarchyPickDir.normalize();
	if (0.0 == fPickLength)
	{
		return 0.0;
	}

	osg::Vec3d vWorldPickTangent = vHierarchyPickDir ^ osg::Vec3d(0, 0, 1);
	vWorldPickTangent.normalize();
	return vWorldPickTangent * vSpeed / fPickLength;
}
