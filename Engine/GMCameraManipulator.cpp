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

#include <iostream>

using namespace GM;

/*************************************************************************
 Macro Defines
*************************************************************************/

#define GM_MAX_DIR_Z				(0.999)
#define GM_HOME_PITCH				(-osg::PI*0.11)
#define GM_GOTO_PITCH				(-osg::PI*0.05)
#define GM_HOME_RADIUS				(9.0)
#define GM_HOME_DURATION			(7.0)
#define GM_GOTO_DURATION			(11.0)
#define GM_RADIUS_SEE_SOLAR			(1e12)		// 观察太阳系的最佳距离，单位：米
#define GM_MAX_DISTANCE				(GM_MAX_RADIUS*2)
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
	m_fCameraRadius(GM_HOME_RADIUS), m_fCameraStartRadius(GM_HOME_RADIUS),
	m_fRadiusAcceleration(0.0), m_fRadiusWeakRatio(3),
	m_fFreeYawSpeed(osg::inDegrees(0.6)), m_fYawSpeed(osg::inDegrees(0.0)), m_fPitchSpeed(0.0),
	m_fTerminalPitch(GM_HOME_PITCH), m_fTerminalYaw(0.0), m_fOriginPitch(GM_HOME_PITCH), m_fOriginYaw(0.0),
	m_fSpinAcceleration(0.0), m_fSpinWeakRatio(4), m_vMoveOnSurfaceSpeed(0.0, 0.0), m_vMix(1.3*GM_GOTO_DURATION, 0.99),
	m_vEyeDeltaMove(0, 0, 0), m_vEyeUp(0, 0, 1), m_vEyeFront(1, 0, 0), m_vEyeLeft(0,1,0),
	m_vTargetHierarchyPos(0, 0, 0),
	m_vTerminalHierarchyPos(0, 0, 0), m_vOriginHierarchyPos(0, 0, 0),
	m_vLastMouseHierarchyPos(0, 0, 0),
	m_vStarWorldPos(0, 0, 0), 
	m_vMousePushScreenPos(0.0f, 0.0f), m_vMouseLastScreenPos(0.0f,0.0f),
	m_vMouseDragLastScreenPos(0.0f, 0.0f),
	m_eManipulator(EGMM_WATCH), m_eWatchState(EGMC_WATCH_FREE),
	m_bEdit(false), m_bEditAudio(false),
	m_bTarget2CelestialBody(false), m_bDoubleClick(false),
	m_vPlanetSolarPos(0, 0, 0), m_fPlanetPeriod(0.0)
{
	m_vEyeFront = osg::Vec3d(0.0, std::cos(m_fOriginPitch), std::sin(m_fOriginPitch));
	m_vTranslate = _GetTranslate();

	osg::Matrix mCameraRotate;
	mCameraRotate.makeLookAt(osg::Vec3d(0, 0, 0), m_vEyeFront, m_vEyeUp);
	m_qRotate.set(mCameraRotate);
}

bool
CGMCameraManipulator::screen2Hierarchy(const float x, const float y, osg::View* theView, osg::Vec3d& vHieOutPos) const
{
	osgViewer::View* view = dynamic_cast<osgViewer::View*>(theView);
	if (!view) return false;

	const osg::Camera* camera = view->getCamera();
	if (!camera) return false;

	osg::Matrix matrix = camera->getViewMatrix();
	if (matrix.isNaN())  return false;
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

	bool bCross = false;
	int iHie = GM_ENGINE.GetHierarchy();
	if (2 <= iHie)
	{
		//不与XY平面相交，无法计算相交点
		if (startVertex.z()*endVertex.z() >= 0)
		{
			osg::Vec3d vDir = endVertex - startVertex;
			vDir.normalize();
			// 将目标距离定为最小半径的2倍
			vHieOutPos = m_vTranslate + vDir * GM_MIN_RADIUS * 2;
			bCross = false;
		}
		else
		{
			double fRatio = abs(startVertex.z() / max(1e-20, abs(endVertex.z() - startVertex.z())));
			vHieOutPos = osg::Vec3d(
				startVertex.x() + fRatio * (endVertex.x() - startVertex.x()),
				startVertex.y() + fRatio * (endVertex.y() - startVertex.y()),
				0);
			// 如果在第三层级，且鼠标位置不在柯伊伯带以内，则将“目标距离”设定为与“当前恒星距离”相同
			double fRadius = vHieOutPos.length();
			if ((3 == iHie) && (fRadius > 0.01))
			{
				osg::Vec3d vDir = endVertex - startVertex;
				vDir.normalize();
				// 将“目标距离”设定为与“当前恒星距离”相同
				vHieOutPos = m_vTranslate + vDir * m_vTranslate.length();
			}
			bCross = true;
		}

		// 如果在第2层级，且鼠标与天体的距离比较小，
		// 则将“目标距离”设定为“当前天体最近的切面的距离”
		if (2 == iHie)
		{	
			double fRadius = vHieOutPos.length();
			double fHieCelestialRadius = GM_ENGINE.GetCelestialMeanRadius() / GM_ENGINE.GetUnit();
			if(fRadius < fHieCelestialRadius)
			{
				osg::Vec3d vDir = endVertex - startVertex;
				vDir.normalize();
				// 将“目标距离”设定为与“当前恒星距离”相同
				vHieOutPos = m_vTranslate + vDir * (m_vTranslate.length() - fHieCelestialRadius);
			}
		}
	}
	else
	{
		osg::Vec3d vDir = endVertex - startVertex;
		vDir.normalize();
		// 将“目标距离”设定为与“当前天体距离”相同
		vHieOutPos = m_vTranslate + vDir * m_vTranslate.length();
		bCross = true;
	}
	return bCross;
}

void CGMCameraManipulator::ChangeCenter(const SGMVector3& vDeltaPos)
{
	osg::Vec3d vDeltaHiePos = _GM2OSG(vDeltaPos)/ GM_ENGINE.GetUnit();

	m_vTargetHierarchyPos -= vDeltaHiePos;
	m_vTerminalHierarchyPos -= vDeltaHiePos;
	m_vOriginHierarchyPos -= vDeltaHiePos;
	m_vTranslate -= vDeltaHiePos;
}

bool CGMCameraManipulator::UpdateHierarchy()
{
	// 空间层级切换
	// 注意：空间层级每隔5个数量级就切换一次
	// 但上切换和下切换的尺度相差一半，最大半径和最小半径的比值刻意设置成2e5
	// 上述比值是为了避免来回切换，不要修改它！！！！
	// 当相机半径小于等于最小半径（5e-4,本空间下）时，则切换到下一层级空间
	// 当相机半径大于等于最大半径（1e2,本空间下）时，则切换到上一层级空间
	// 半径 <= 5e21，5跳4	半径 >= 1e22，4跳5
	// 半径 <= 5e16，4跳3	半径 >= 1e17，3跳4
	// 半径 <= 5e11，3跳2	半径 >= 1e12，2跳3
	// 半径 <= 5e6，2跳1	半径 >= 1e7，1跳2
	SGMVector3 vTranslate = _OSG2GM(m_vTranslate);
	SGMVector3 vTargetHiePos = _OSG2GM(m_vTargetHierarchyPos);

	if (m_fCameraRadius <= GM_MIN_RADIUS)
	{
		if (GM_ENGINE.SubHierarchy(vTranslate, vTargetHiePos))
		{
			int iHie = GM_ENGINE.GetHierarchy();
			if (2 == iHie)
			{
				// 世界中心点在行星上
				SGMVector3 vPlanetPos;
				GM_ENGINE.GetCelestialBody(vPlanetPos, m_fPlanetPeriod);
				m_vPlanetSolarPos = _GM2OSG(vPlanetPos);
			}
			if (1 == iHie)
			{
				osg::Matrix mCameraRotate;
				m_qRotate.get(mCameraRotate);
				// 修改姿态
				osg::Vec3d vRotateEye, vFront, vUp; //相机的旋转姿态，和相机的位置无关
				mCameraRotate.getLookAt(vRotateEye, vFront, vUp);//这里的vRotateEye不是实际相机的位置，而是0，0，0
				m_vEyeFront = _GM2OSG(GM_ENGINE.AfterSubHierarchy(_OSG2GM(osg::Vec4d(vFront, 0.0))));
				m_vEyeFront.normalize();
				m_vEyeUp = _GM2OSG(GM_ENGINE.AfterSubHierarchy(_OSG2GM(osg::Vec4d(vUp, 0.0))));
				m_vEyeUp.normalize();
				m_vEyeLeft = m_vEyeUp ^ m_vEyeFront;
				m_vEyeLeft.normalize();

				mCameraRotate.makeLookAt(vRotateEye, m_vEyeFront, m_vEyeUp);
				m_qRotate.set(mCameraRotate);
			}

			switch (iHie)
			{
			case 6:
			case 5:
			case 4:// 从第4层级以上往下跳跃：6->5->4
			{
				m_vTerminalHierarchyPos = m_vStarWorldPos / GM_ENGINE.GetUnit();
				m_vOriginHierarchyPos = m_vOriginHierarchyPos * GM_UNIT_SCALE;
			}
			break;
			case 3:// 4->3
			case 2:// 3->2
			case 1:// 2->1
			case 0:// 1->0
			{
				m_vTerminalHierarchyPos = _GM2OSG(GM_ENGINE.AfterSubHierarchy(_OSG2GM(osg::Vec4d(m_vTerminalHierarchyPos,1.0))));
				m_vOriginHierarchyPos = _GM2OSG(GM_ENGINE.AfterSubHierarchy(_OSG2GM(osg::Vec4d(m_vOriginHierarchyPos,1.0))));
			}
			break;
			default:
				break;
			}

			m_vTargetHierarchyPos = _GM2OSG(GM_ENGINE.AfterSubHierarchy(_OSG2GM(osg::Vec4d(m_vTargetHierarchyPos, 1.0))));
			m_vTranslate = _GM2OSG(GM_ENGINE.AfterSubHierarchy(_OSG2GM(osg::Vec4d(m_vTranslate, 1.0))));

			m_fCameraRadius *= GM_UNIT_SCALE;
			m_fCameraStartRadius *= GM_UNIT_SCALE;

			return true;
		}
		else
		{
			if (0 == GM_ENGINE.GetHierarchy())
			{
				m_fCameraRadius = GM_MIN_RADIUS * 1.001;
			}
		}
	}
	else if (m_fCameraRadius >= GM_MAX_RADIUS && GM_HIERARCHY_MAX > GM_ENGINE.GetHierarchy())
	{
		if (GM_ENGINE.AddHierarchy(vTranslate, vTargetHiePos))
		{
			int iHie = GM_ENGINE.GetHierarchy();
			if (2 == iHie)
			{
				osg::Matrix mCameraRotate;
				m_qRotate.get(mCameraRotate);
				// 修改姿态
				osg::Vec3d vRotateEye, vFront, vUp; //相机的旋转姿态，和相机的位置无关
				mCameraRotate.getLookAt(vRotateEye, vFront, vUp);//这里的vRotateEye不是实际相机的位置，而是0，0，0
				m_vEyeFront = _GM2OSG(GM_ENGINE.AfterAddHierarchy(_OSG2GM(osg::Vec4d(vFront, 0.0))));
				m_vEyeFront.normalize();
				m_vEyeUp = _GM2OSG(GM_ENGINE.AfterAddHierarchy(_OSG2GM(osg::Vec4d(vUp, 0.0))));
				m_vEyeUp.normalize();
				m_vEyeLeft = m_vEyeUp ^ m_vEyeFront;
				m_vEyeLeft.normalize();
				mCameraRotate.makeLookAt(vRotateEye, m_vEyeFront, m_vEyeUp);
				m_qRotate.set(mCameraRotate);
			}
			if (3 == iHie)
			{
				// 世界中心点在行星上
				SGMVector3 vPlanetPos;
				GM_ENGINE.GetCelestialBody(vPlanetPos, m_fPlanetPeriod);
				m_vPlanetSolarPos = _GM2OSG(vPlanetPos);
			}
			if ((4 == iHie) && (EGMC_WATCH_HOME == m_eWatchState || EGMC_WATCH_JUMP == m_eWatchState))
			{
				m_vTerminalHierarchyPos = osg::Vec3d(0, 0, 0);
			}

			m_vTerminalHierarchyPos = _GM2OSG(GM_ENGINE.AfterAddHierarchy(_OSG2GM(osg::Vec4d(m_vTerminalHierarchyPos,1.0))));
			m_vOriginHierarchyPos = _GM2OSG(GM_ENGINE.AfterAddHierarchy(_OSG2GM(osg::Vec4d(m_vOriginHierarchyPos,1.0))));
			m_vTargetHierarchyPos = _GM2OSG(GM_ENGINE.AfterAddHierarchy(_OSG2GM(osg::Vec4d(m_vTargetHierarchyPos, 1.0))));
			m_vTranslate = _GM2OSG(GM_ENGINE.AfterAddHierarchy(_OSG2GM(osg::Vec4d(m_vTranslate, 1.0))));

			m_fCameraRadius /= GM_UNIT_SCALE;
			m_fCameraStartRadius /= GM_UNIT_SCALE;

			return true;
		}
		else
		{
			if (6 == GM_ENGINE.GetHierarchy())
			{
				m_fCameraRadius = GM_MAX_RADIUS * 0.999;
			}
		}
	}
	else
	{
		return false;
	}

	return false;
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
	// 更新当前帧鼠标射线在银盘面上的层级空间坐标，如果未命中，则不更新
	osg::Vec3d vMouseHierarchyPos = m_vLastMouseHierarchyPos;
	// 焦点是否在银盘或者太阳系黄道面上
	bool bInGalaxy = screen2Hierarchy(ea.getX(), ea.getY(), aa.asView(), vMouseHierarchyPos);
	double fUnit = GM_ENGINE.GetUnit();

	switch (ea.getEventType())
	{
	case (osgGA::GUIEventAdapter::FRAME):
	{
		double timeCurrFrame = osg::Timer::instance()->time_s();
		double fDeltaTime = timeCurrFrame - m_fTimeLastFrame;

		float fInnerDeltaTime = fDeltaTime;
		fInnerDeltaTime += m_fDeltaStep;
		while (fInnerDeltaTime >= m_fConstantStep)
		{
			_InnerUpdate(m_fConstantStep);
			fInnerDeltaTime -= m_fConstantStep;
		}
		m_fDeltaStep = fInnerDeltaTime;

		// 客观事实的更新函数，与飞行模式无关的更新都放在这里，
		// 例如：行星的公转自转
		_ObjectiveUpdate(ea, aa);
		// 各个飞行模式下需要单独更新的内容都放在这里
		_FlyUpdate(ea, aa);

		// 如果焦点在银河系盘面上
		if (bInGalaxy)
		{
			GM_ENGINE.SetMousePosition(_OSG2GM(vMouseHierarchyPos));

			if (m_bEdit && EGMC_WATCH_ZOOM != m_eWatchState && EGMC_WATCH_MOVE != m_eWatchState)
			{
				if ((vMouseHierarchyPos - m_vStarWorldPos / fUnit).length() < GM_HANDLE_RADIUS)
				{
					GM_ENGINE.SetHandleHover(true);
				}
				else
				{
					GM_ENGINE.SetHandleHover(false);
				}
			}
		}

		if (EGMM_WATCH == m_eManipulator)
		{
			m_vEyeDeltaMove *= exp2(-fDeltaTime * 7.0);
			m_fPitchSpeed *= exp2(-fDeltaTime * 10.0);
			m_fSpinAcceleration *= exp2(-fDeltaTime * 8);

			m_vMoveOnSurfaceSpeed *= exp2(-fDeltaTime * 20.0);
		}

		m_fTimeLastFrame = timeCurrFrame;
		m_vMouseLastScreenPos = osg::Vec2f(ea.getX(), ea.getY());
	}
	break;
	case (osgGA::GUIEventAdapter::PUSH):
	{
		m_vMousePushScreenPos = osg::Vec2f(ea.getX(), ea.getY());

		switch (ea.getButton())
		{
		case (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON):
		{
			if (m_bEdit &&
				(vMouseHierarchyPos - m_vStarWorldPos / fUnit).length() < GM_HANDLE_RADIUS)
			{
				m_bEditAudio = true;
			}

			int iHie = GM_ENGINE.GetHierarchy();
			if (3 == iHie)
			{
				SGMVector3 vPlanetHiePos;
				m_bTarget2CelestialBody = GM_ENGINE.GetNearestCelestialBody(_OSG2GM(vMouseHierarchyPos),
					vPlanetHiePos, m_fPlanetPeriod);
				if (m_bTarget2CelestialBody)
				{
					m_vPlanetSolarPos = _GM2OSG(vPlanetHiePos) * fUnit;
				}
			}

			if (EGMM_DRIVE == m_eManipulator)
			{
				_SetPitchYaw(ea.getXnormalized(), ea.getYnormalized());
			}
		}
		break;
		case (osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON):
		{
			// 在0，1层级，中间改成在星球表面水平移动，所以不进入move模式
			if (2 <= GM_ENGINE.GetHierarchy())
			{
				_SetWatchState(EGMC_WATCH_MOVE);
			}
		}
		break;
		case (osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON):
		{
			if (!m_bEdit)
			{
				m_fRadiusWeakRatio = 20;
				m_fSpinWeakRatio = 20;
			}
		}
		break;
		default:
			break;
		}

		m_vMouseDragLastScreenPos = m_vMousePushScreenPos;
	}
	break;
	case (osgGA::GUIEventAdapter::RELEASE):
	{
		m_sDrive.bPitchOrYaw = false;

		switch (ea.getButton())
		{
		case (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON):
		case (osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON):
		{
			int iHie = GM_ENGINE.GetHierarchy();
			if (!((3 == iHie) && m_bDoubleClick && m_bTarget2CelestialBody))
			{
				_SetWatchState(EGMC_WATCH_FREE);
			}
			m_bEditAudio = false;
			GM_ENGINE.SetCapture(false);
			if (m_bEdit)
			{
				m_vStarWorldPos = _GM2OSG(GM_ENGINE.GetCurrentStarWorldPos());
			}
		}
		break;
		case (osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON):
		{
			m_fRadiusWeakRatio = 3;
			m_fSpinWeakRatio = 4;
		}
		break;
		default:
			break;
		}
		m_bDoubleClick = false;
	}
	break;
	case (osgGA::GUIEventAdapter::DRAG):
	{
		float fDeltaX = ea.getX() - m_vMouseDragLastScreenPos.x();
		float fDeltaY = ea.getY() - m_vMouseDragLastScreenPos.y();

		if (EGMM_DRIVE == m_eManipulator)
		{
			switch (ea.getButtonMask())
			{
			case (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON):
			{
				_SetPitchYaw(ea.getXnormalized(), ea.getYnormalized());
			}
			break;
			default:
				break;
			}
		}
		else
		{
			switch (ea.getButtonMask())
			{
			case (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON):
			{
				if (m_bEditAudio)
				{
					GM_ENGINE.SetCapture(true);
				}
				else
				{
					if (EGMC_WATCH_ZOOM != m_eWatchState)
					{
						_SetWatchState(EGMC_WATCH_ZOOM);
					}
				}
			}
			break;
			case (osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON):
			{
				double fMoveSpeed = m_fCameraRadius * fUnit * 2e-9;
				m_vMoveOnSurfaceSpeed -= osg::Vec2(fDeltaX, fDeltaY) * fMoveSpeed;
			}
			break;
			case (osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON):
			{
				if (fDeltaX == 0.0f || 1.0f < std::abs(fDeltaY / fDeltaX)) // 控制俯仰
				{
					m_fPitchSpeed = (m_fPitchSpeed - fDeltaY * 0.09) * 0.8;
				}
				else // 控制自传
				{
					m_fSpinAcceleration = -0.04 * fDeltaX;
				}
			}
			break;
			default:
				break;
			}
		}

		m_vMouseDragLastScreenPos = osg::Vec2f(ea.getX(), ea.getY());
	}
	break;
	case (osgGA::GUIEventAdapter::SCROLL):
	{
		if (EGMC_WATCH_FREE == m_eWatchState)
		{
			switch (ea.getScrollingMotion())
			{
			case osgGA::GUIEventAdapter::SCROLL_UP:
			{
				m_fCameraRadius *= 0.8;
			}
			break;
			case osgGA::GUIEventAdapter::SCROLL_DOWN:
			{
				m_fCameraRadius *= 1.2;
			}
			break;
			}
		}
	}
	break;
	case (osgGA::GUIEventAdapter::KEYDOWN):
	{
		int iHie = GM_ENGINE.GetHierarchy();

		switch (ea.getKey())
		{
		//case osgGA::GUIEventAdapter::KEY_5:
		//{
		//	// 太阳位置
		//	GM_ENGINE.SetAudio(0.0, -2.5e20);
		//	int iHie = GM_ENGINE.GetHierarchy();
		//	if (4 == iHie)
		//	{
		//		if (m_fCameraRadius >= GM_HOME_RADIUS)
		//		{
		//			_SetWatchState(EGMC_WATCH_GOTO);
		//		}
		//		else
		//		{
		//			_SetWatchState(EGMC_WATCH_JUMP);
		//		}
		//	}
		//	else if (4 > iHie)
		//	{
		//		_SetWatchState(EGMC_WATCH_JUMP);
		//	}
		//	else
		//	{
		//		// to do
		//	}
		//}
		//break;
		case 'A':
		case 'a':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				bool bShift = (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_SHIFT) || m_sDrive.bShift;
				m_sDrive.fAccelerationLeft = bShift ?
					m_sDrive.vAcceleration_Max.x() : m_sDrive.vAcceleration_Normal.x();
			}
		}
		break;
		case 'D':
		case 'd':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				bool bShift = (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_SHIFT) || m_sDrive.bShift;
				m_sDrive.fAccelerationRight = bShift ?
					m_sDrive.vAcceleration_Max.x() : m_sDrive.vAcceleration_Normal.x();
			}
		}
		break;
		case 'E':
		case 'e':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.bRollRight = true;
			}
		}
		break;
		case 'Q':
		case 'q':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.bRollLeft = true;
			}
		}
		break;
		case 'S':
		case 's':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				bool bShift = (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_SHIFT) || m_sDrive.bShift;
				m_sDrive.fAccelerationBack = bShift ?
					m_sDrive.vAcceleration_Max.z() : m_sDrive.vAcceleration_Normal.z();
			}
		}
		break;
		case 'W':
		case 'w':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				bool bShift = (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_SHIFT) || m_sDrive.bShift;
				m_sDrive.fAccelerationFront = bShift ?
					m_sDrive.vAcceleration_Max.z() : m_sDrive.vAcceleration_Normal.z();
			}
		}
		break;
		case 'X':
		case 'x':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.bBrakeMode = true;
			}
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Space:
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				bool bShift = (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_SHIFT) || m_sDrive.bShift;
				m_sDrive.fAccelerationUp = bShift ?
					m_sDrive.vAcceleration_Max.y() : m_sDrive.vAcceleration_Normal.y();
			}
			else
			{
				if (1 < iHie)
				{
					_SetWatchState(EGMC_WATCH_HOME);
				}
			}
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Return:
		{
			_SetWatchState(EGMC_WATCH_GOTO);
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F2:
		{
			GM_ENGINE.SetPlayMode(EGMA_MOD_CIRCLE);
			GM_ENGINE.Next();

			if (4 == iHie)
			{
				if (m_fCameraRadius >= GM_HOME_RADIUS)
				{
					_SetWatchState(EGMC_WATCH_GOTO);
				}
				else
				{
					_SetWatchState(EGMC_WATCH_JUMP);
				}
			}
			else if (4 > iHie)
			{
				_SetWatchState(EGMC_WATCH_JUMP);
			}
			else
			{
				// to do
			}
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F3:
		{
			GM_ENGINE.SetPlayMode(EGMA_MOD_RANDOM);
			GM_ENGINE.Next();

			int iHie = GM_ENGINE.GetHierarchy();
			if (4 == iHie)
			{
				if (m_fCameraRadius >= GM_HOME_RADIUS)
				{
					_SetWatchState(EGMC_WATCH_GOTO);
				}
				else
				{
					_SetWatchState(EGMC_WATCH_JUMP);
				}
			}
			else if (4 > iHie)
			{
				_SetWatchState(EGMC_WATCH_JUMP);
			}
			else
			{
				// to do
			}
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F4:
		{
			// 防止每次修改完退出编辑模式时，自动跳转视点
			if (m_bEdit)
			{
				m_vStarWorldPos = _GM2OSG(GM_ENGINE.GetCurrentStarWorldPos());
			}

			m_bEdit = !m_bEdit;
			GM_ENGINE.SetEditMode(m_bEdit);
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F5:
		{
			GM_ENGINE.Load();
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F6:
		{
			_SetManipulatorMode(EGMM_DRIVE);
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F7:
		{
			_SetManipulatorMode(EGMM_WATCH);
		}
		break;
		case osgGA::GUIEventAdapter::KEY_F9:
		{
			_SetWatchState(EGMC_WATCH_ULTIMATE);
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Shift_L:
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.bShift = true;
			}
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Control_L:
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				bool bShift = (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_SHIFT) || m_sDrive.bShift;
				m_sDrive.fAccelerationDown = bShift ?
					m_sDrive.vAcceleration_Max.y() : m_sDrive.vAcceleration_Normal.y();
			}
		}
		break;
		case 19:
		{
			// ctrl + s
			GM_ENGINE.Save();
			// 记录太阳系此刻的信息，保证重启时太阳系行星的同步
			GM_ENGINE.SaveSolarData();
		}
		break;
		default:
			break;
		}

		GM_SYSTEM_MANAGER.GMKeyDown(EGMKeyCode(ea.getKey()));
	}
	break;
	case (osgGA::GUIEventAdapter::KEYUP):
	{
		switch (ea.getKey())
		{
		case 'A':
		case 'a':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.fAccelerationLeft = 0;
			}
		}
		break;
		case 'D':
		case 'd':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.fAccelerationRight = 0;
			}
		}
		break;
		case 'E':
		case 'e':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.bRollRight = false;
			}
		}
		break;
		case 'Q':
		case 'q':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.bRollLeft = false;
			}
		}
		break;
		case 'S':
		case 's':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.fAccelerationBack = 0;
			}
		}
		break;
		case 'W':
		case 'w':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.fAccelerationFront = 0;
			}
		}
		break;
		case 'X':
		case 'x':
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.bBrakeMode = false;
			}
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Up:
		case osgGA::GUIEventAdapter::KEY_Down:
		{
			//m_fPitchSpeed = 0;
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Space:
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.fAccelerationUp = 0;
			}
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Shift_L:
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.bShift = false;
			}
		}
		break;
		case osgGA::GUIEventAdapter::KEY_Control_L:
		{
			if (EGMM_DRIVE == m_eManipulator)
			{
				m_sDrive.fAccelerationDown = 0;
			}
		}
		break;
		default:
			break;
		}

		GM_SYSTEM_MANAGER.GMKeyUp(EGMKeyCode(ea.getKey()));
	}
	break;
	case (osgGA::GUIEventAdapter::DOUBLECLICK):
	{
		m_bDoubleClick = true;
		if (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON == ea.getButtonMask())
		{
			if (4 == GM_ENGINE.GetHierarchy())
			{
				SGMVector3 vMouseWorldPos = _OSG2GM(vMouseHierarchyPos) * GM_ENGINE.GetUnit();
				GM_ENGINE.SetAudio(vMouseWorldPos.x, vMouseWorldPos.y);
			}
			if (3 == GM_ENGINE.GetHierarchy() || 2 == GM_ENGINE.GetHierarchy())
			{
				if (m_bTarget2CelestialBody)
				{
					_SetWatchState(EGMC_WATCH_GOTO);
				}
			}
		}
	}
	break;
	default:
		break;
	}

	m_vLastMouseHierarchyPos = vMouseHierarchyPos;

	return false;
}

CGMCameraManipulator::~CGMCameraManipulator()
{
}

void CGMCameraManipulator::_InnerUpdate(const float updateStepTime)
{
	osg::Vec3d vNewStarWorldPos = _GM2OSG(GM_ENGINE.GetCurrentStarWorldPos());

	if (m_vStarWorldPos != vNewStarWorldPos)
	{
		m_vStarWorldPos = vNewStarWorldPos;
		if (!m_bEdit)
		{	
			if (EGMC_WATCH_FREE == m_eWatchState)
			{
				int iHie = GM_ENGINE.GetHierarchy();
				if (4 == iHie)
				{
					if (m_fCameraRadius >= GM_HOME_RADIUS)
					{
						_SetWatchState(EGMC_WATCH_GOTO);
					}
					else
					{
						_SetWatchState(EGMC_WATCH_JUMP);
					}
				}
				else if (4 > iHie)
				{
					_SetWatchState(EGMC_WATCH_JUMP);
				}
				else
				{
					// to do
				}
			}
		}
	}
}

void CGMCameraManipulator::_ObjectiveUpdate(const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa)
{
	double timeCurrFrame = osg::Timer::instance()->time_s();
	double fDeltaTime = timeCurrFrame - m_fTimeLastFrame;
	int iHie = GM_ENGINE.GetHierarchy();

	if ((3 == iHie) && 0 != m_fPlanetPeriod)
	{
		double fPlanetDeltaAngle = -osg::PI * 2 * fDeltaTime / m_fPlanetPeriod;
		double cosA = cos(fPlanetDeltaAngle);
		double sinA = sin(fPlanetDeltaAngle);
		osg::Matrixd planteRotateMatrix(
			cosA, -sinA, 0, 0,
			sinA, cosA, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		// 更新当前跟踪的行星在公转轨道上的位置
		m_vPlanetSolarPos = planteRotateMatrix.preMult(m_vPlanetSolarPos);
	}

	// 更新 m_fCursorStillTime
	if (osg::Vec2f(ea.getX(), ea.getY()) == m_vMouseLastScreenPos)
	{
		m_fCursorStillTime += fDeltaTime;
	}
	else
	{
		if (m_fCursorStillTime > GM_CURSOR_TIME)
		{
			m_fCursorStillTime = -0.001f;
		}
	}
	//// 显示或隐藏鼠标,但三维窗口不成功，暂时关闭这个功能
	//if ((m_fCursorStillTime - fDeltaTime) < GM_CURSOR_TIME && m_fCursorStillTime > GM_CURSOR_TIME)
	//{
	//	GM_SYSTEM_MANAGER.SetCursorVisible(false);
	//}
	//else
	//{
	//	if (m_fCursorStillTime < 0.0f)
	//	{
	//		GM_SYSTEM_MANAGER.SetCursorVisible(true);
	//	}
	//}
}

void CGMCameraManipulator::_FlyUpdate(const osgGA::GUIEventAdapter & ea, osgGA::GUIActionAdapter & aa)
{
	double timeCurrFrame = osg::Timer::instance()->time_s();
	double fDeltaTime = timeCurrFrame - m_fTimeLastFrame;
	int iHie = GM_ENGINE.GetHierarchy();
	double fUnit = GM_ENGINE.GetUnit();
	osg::Matrix mCameraRotate;
	m_qRotate.get(mCameraRotate);

	switch (m_eWatchState)
	{
	case EGMC_WATCH_FREE:
	case EGMC_WATCH_ZOOM:
	case EGMC_WATCH_MOVE:
	{
		// 修改姿态
		osg::Vec3d vRotateEye, vRotateCenter, m_vEyeUp; //相机的旋转姿态，和相机的位置无关
		mCameraRotate.getLookAt(vRotateEye, vRotateCenter, m_vEyeUp);//这里的vRotateEye不是实际相机的位置，而是0，0，0
		m_vEyeFront = vRotateCenter - vRotateEye; //相机视点方向
		m_vEyeFront.normalize();
		m_vEyeLeft = m_vEyeUp ^ m_vEyeFront;
		m_vEyeLeft.normalize();

		switch (m_eWatchState)
		{
		case EGMC_WATCH_FREE:
		{
			m_fRadiusAcceleration *= exp2(-fDeltaTime * m_fRadiusWeakRatio);
			m_fFreeYawSpeed = _Mix(m_fFreeYawSpeed, osg::inDegrees(0.6), min(1.0, fDeltaTime));
		}
		break;
		case EGMC_WATCH_ZOOM:
		{
			float fDeltaY_Zoom = ea.getY() - m_vMousePushScreenPos.y();
			float fSignY = osg::sign(fDeltaY_Zoom);
			// 目标点是真空
			if (_IsTargetInVacuum(iHie) && (fSignY < 0))
			{
				double fEye2MinVacuum = max(0.0, m_fCameraRadius - GM_MIN_RADIUS * 2);
				m_fRadiusAcceleration = fEye2MinVacuum * fSignY * fDeltaTime * min(4.0, abs(fDeltaY_Zoom) * 0.004);
			}
			else
			{
				m_fRadiusAcceleration = m_fCameraRadius * fSignY * fDeltaTime * min(4.0, abs(fDeltaY_Zoom) * 0.004);
			}
			m_fFreeYawSpeed = 0.0;
		}
		break;
		case EGMC_WATCH_MOVE:
		{
			m_fRadiusAcceleration = 0.0;
			m_fFreeYawSpeed = 0.0;

			if (4 >= iHie)
			{
				float vMouseMoveX = ea.getX() - m_vMouseLastScreenPos.x();
				float vMouseMoveY = ea.getY() - m_vMouseLastScreenPos.y();
				if (vMouseMoveX != 0 || vMouseMoveY != 0)
				{
					m_vEyeDeltaMove = (m_vEyeUp * vMouseMoveY - m_vEyeLeft * vMouseMoveX)* 0.05 * fDeltaTime * m_fCameraRadius;
				}
			}
		}
		break;
		default: 
			break;
		}

		// 眼点在当前空间层级下的坐标
		osg::Vec3d vEyeHierarchyPos = m_vTargetHierarchyPos - m_vEyeFront * m_fCameraRadius;

		if (EGMM_WATCH == m_eManipulator)
		{
			if (1 == iHie)
			{
				/*
				* 如果在第1空间层级,目标点始终围绕在以相机为球心的球面上
				* 球面半径根据实际情况动态改变，但不会陷入天体而穿地
				*/
				_IsIntoGround(m_vTerminalHierarchyPos);
			}
			else if (2 == iHie)
			{
				// 如果在第2空间层级,视线中点接触到了天体，就会自动锁定在目标天体周围
				_IsIntoGround(m_vTerminalHierarchyPos);
			}
			else {}
		}

		// 缩放的同时根据鼠标位置调整目标点位置，提升用户体验的非常重要的一部分
		double fNextCameraRadius = m_fCameraRadius;
		if (m_fRadiusAcceleration != 0)
		{
			osg::Vec3d vEye2Terminal = m_vTerminalHierarchyPos - vEyeHierarchyPos;
			fNextCameraRadius = osg::clampBetween(m_fCameraRadius + m_fRadiusAcceleration, GM_MIN_RADIUS, GM_MAX_DISTANCE);
			// 根据鼠标位置调整目标点位置
			osg::Vec3d vNextEyeHierarchyPos = m_vTerminalHierarchyPos - vEye2Terminal * fNextCameraRadius / m_fCameraRadius;
			m_vTargetHierarchyPos = vNextEyeHierarchyPos + m_vEyeFront * fNextCameraRadius;
		}

		// 在第四层级空间，远离星系时，将目标点移动到原点上
		const double fStartSmoothR = GM_MAX_RADIUS * 0.01;
		const double fEndSmoothR = GM_MAX_RADIUS;
		if ((4 == iHie) && (m_fRadiusAcceleration > 0.0) && (m_fCameraRadius > fStartSmoothR))
		{
			double fFact = _Mix(0.1, 10.0, (m_fCameraRadius - fStartSmoothR) / (fEndSmoothR - fStartSmoothR));
			m_vTargetHierarchyPos *= 1 - min(1, fFact*m_fRadiusAcceleration / max(1e-40, fEndSmoothR - m_fCameraRadius));
		}
		// 根据鼠标的拖拽，移动目标点
		m_vTargetHierarchyPos -= m_vEyeDeltaMove;
		// 更新相机目标半径
		m_fCameraRadius = fNextCameraRadius;

		// 相机在星球上操作方式和太空不同，必须手动过渡
		double fMoveX = 0.0;
		double fMoveY = m_fPitchSpeed;
		// [0,1], 0表示在太空自由操作，1表示在行星表面受约束
		double fSurfaceFact = 0.0;
		if (2 == iHie)
		{
			const double fStartR = GM_MIN_RADIUS;
			fSurfaceFact = osg::clampBetween(2 - m_fCameraRadius / fStartR, 0.0, 1.0);
			fMoveX = m_vMoveOnSurfaceSpeed.x() * fSurfaceFact;
			fMoveY = _Mix(m_fPitchSpeed, m_vMoveOnSurfaceSpeed.y(), fSurfaceFact);
		}
		else if (1 == iHie)
		{
			const double fStartR = GM_MAX_RADIUS;
			fSurfaceFact = osg::clampBetween((1 - m_fCameraRadius / fStartR) * 2, 0.0, 1.0);
			fMoveX = m_vMoveOnSurfaceSpeed.x() * fSurfaceFact;
			fMoveY = _Mix(m_fPitchSpeed, m_vMoveOnSurfaceSpeed.y(), fSurfaceFact);
		}
		else if (0 == iHie)
		{
			fMoveX = m_vMoveOnSurfaceSpeed.x();
			fMoveY = m_vMoveOnSurfaceSpeed.y();
		}
		else {}

		osg::Matrix mDeltaRotate = osg::Matrix::rotate(
			fMoveX * fDeltaTime, m_vEyeUp,
			fMoveY * fDeltaTime, m_vEyeLeft,
			0.0 * fDeltaTime, m_vEyeFront);

		// 更新偏转速度
		m_fYawSpeed = osg::clampBetween(m_fYawSpeed + m_fSpinAcceleration, -5.0, 5.0);
		m_fYawSpeed *= exp2(-fDeltaTime * m_fSpinWeakRatio);
		m_fYawSpeed *= 1 - fSurfaceFact; // 在星球表面就不绕地轴偏转

		// 判断俯仰角是否超过界限，如果没超过就设置水平和垂直旋转的角度
		if (((m_vEyeFront.z() < GM_MAX_DIR_Z) && (m_fPitchSpeed <= 0)) ||
			((m_vEyeFront.z() > -GM_MAX_DIR_Z) && (m_fPitchSpeed >= 0)))
		{
			osg::Matrix mDeltaSpin = osg::Matrix::rotate((m_fYawSpeed + m_fFreeYawSpeed) * fDeltaTime, osg::Vec3d(0, 0, 1));
			// 小于等于第二层级时，取消相机的固定水平旋转
			if (2 >= iHie)
			{
				mDeltaSpin = osg::Matrix::rotate(m_fYawSpeed * fDeltaTime, osg::Vec3d(0, 0, 1));

				// 自由模式，012级空间下
				if (EGMC_WATCH_FREE == m_eWatchState)
				{
					// 需要修改相机旋转算法，保证天体在屏幕上的位置不变
					m_vTargetHierarchyPos = m_vTargetHierarchyPos * mDeltaSpin * mDeltaRotate;

					// 流浪地球——启航镜头
					if (3 == GM_ENGINE.GetCelestialIndex())
					{
						//m_vTargetHierarchyPos -= vRight * 1.5e-6 * (1 - std::exp(-m_fCursorStillTime * m_fCursorStillTime * 0.003));
					}
				}
			}

			m_vEyeFront = m_vEyeFront * mDeltaSpin * mDeltaRotate;
			m_vEyeFront.normalize();
		}
	}
	break;
	case EGMC_WATCH_GOTO:
	case EGMC_WATCH_HOME:
	case EGMC_WATCH_JUMP:
	{
		m_fTimeSinceJump = timeCurrFrame - m_fTimeJumpStart; //单位:秒
		// 半径、角度的混合系数
		double fMix = _GetMix(m_fTimeSinceJump / m_fJumpDuration, m_eWatchState);
		// 目标点混合系数
		double fTargetMix = _Smoothstep(0, 1, fMix);

		// 观察太阳系云的最佳距离，单位：当前空间层级单位长度
		double fSeeSolarBestRadius = GM_RADIUS_SEE_SOLAR / fUnit;

		switch (iHie)
		{
		case 4:
		{
			if (EGMC_WATCH_GOTO == m_eWatchState)
			{
				if (m_fTimeSinceJump > m_fJumpDuration)
				{
					_SetWatchState(EGMC_WATCH_FREE);
				}
				m_fCameraRadius = _Mix(m_fCameraStartRadius, fSeeSolarBestRadius, fMix);
			}
			else
			{
				if (m_fTimeSinceJump > m_fJumpDuration)
				{
					if (EGMC_WATCH_JUMP == m_eWatchState)
					{
						_SetWatchState(EGMC_WATCH_GOTO);
						fMix = 0.0;
						fTargetMix = 0.0;
					}
					else if (EGMC_WATCH_HOME == m_eWatchState)
					{
						_SetWatchState(EGMC_WATCH_FREE);
						fMix = 0.0;
						fTargetMix = 0.0;
					}
					else{}
				}
				m_fCameraRadius = _Mix(m_fCameraStartRadius, GM_HOME_RADIUS, fMix);
			}
		}
		break;
		case 3:
		case 2:
		{
			if (EGMC_WATCH_GOTO == m_eWatchState)
			{
				if (m_fCameraRadius >= fSeeSolarBestRadius)
				{
					if (m_fTimeSinceJump > m_fJumpDuration)
					{
						_SetWatchState(EGMC_WATCH_FREE);
					}
					m_fCameraRadius = _Mix(m_fCameraStartRadius, fSeeSolarBestRadius, fMix);
				}
				else
				{
					if (m_fTimeSinceJump > m_fJumpDuration || m_fCameraRadius <= GM_MIN_RADIUS)
					{
						_SetWatchState(EGMC_WATCH_FREE);
					}
					m_fCameraRadius = _Mix(m_fCameraStartRadius, GM_MIN_RADIUS, fMix);
				}
			}
			else
			{
				m_fCameraRadius = _Mix(m_fCameraStartRadius, GM_HOME_RADIUS * std::pow(GM_UNIT_SCALE, 4 - iHie), fMix);
			}
		}
		break;
		case 1:
		{
			if (EGMM_WATCH == m_eManipulator)
			{
				// 将目标点定在天体表面，避免穿地
				_IsIntoGround(m_vTerminalHierarchyPos);
			}
		}
		break;
		default:
			break;
		}

		// 目标点混合
		m_vTargetHierarchyPos = m_vOriginHierarchyPos * (1 - fTargetMix) + m_vTerminalHierarchyPos * fTargetMix;

		// 修改姿态
		double fTargetYaw = _Mix(m_fOriginYaw, m_fTerminalYaw, fMix);
		double fTargetPitch = _Mix(m_fOriginPitch, m_fTerminalPitch, fMix);

		double fZ = sin(fTargetPitch);
		double fXY = sqrt(1 - fZ * fZ);
		m_vEyeFront = osg::Vec3d(fXY * sin(fTargetYaw), fXY * cos(fTargetYaw), fZ);
		m_vEyeFront.normalize();
	}
	break;
	case EGMC_WATCH_ULTIMATE:
	{
		if (6 == iHie)
		{
			if (m_fCameraRadius >= GM_ULTIMATE_RADIUS)
			{
				_SetWatchState(EGMC_WATCH_FREE);
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

		m_fTimeSinceJump = timeCurrFrame - m_fTimeJumpStart; //单位:秒
		// 半径、角度的混合系数
		double fMix = _GetMix(m_fTimeSinceJump / m_fJumpDuration, EGMC_WATCH_ULTIMATE);
		// 目标点混合系数
		double fTargetMix = _Smoothstep(0, 1, fMix);

		// 修改姿态
		double fTargetYaw = _Mix(m_fOriginYaw, m_fTerminalYaw, fMix);
		double fTargetPitch = _Mix(m_fOriginPitch, m_fTerminalPitch, fMix);

		double fZ = sin(fTargetPitch);
		double fXY = sqrt(1 - fZ * fZ);
		m_vEyeFront = osg::Vec3d(fXY * sin(fTargetYaw), fXY * cos(fTargetYaw), fZ);
		m_vEyeFront.normalize();
	}
	break;
	default:
		break;
	}

	/*
	** 由于在银河系、恒星系、行星系、卫星系之间转换时，需要转换坐标系，让用户有合适的“上”和“下”的感觉，
	** 所以必须考虑不同坐标系的平滑过渡
	*/
	switch (iHie)
	{
	case 0:// to do
	{
	}
	break;
	case 1:// to do
	{
	}
	break;
	case 2:// to do
	{
	}
	break;
	case 3:
	case 4:// to do
	{
		// 小层级空间
		SGMVector3 vGMX_Smaller, vGMY_Smaller, vGMZ_Smaller;
		GM_ENGINE.GetSmallerHierarchyCoord(vGMX_Smaller, vGMY_Smaller, vGMZ_Smaller);
		osg::Vec3d vX_Smaller = _GM2OSG(vGMX_Smaller);
		osg::Vec3d vY_Smaller = _GM2OSG(vGMY_Smaller);
		osg::Vec3d vZ_Smaller = _GM2OSG(vGMZ_Smaller);
		osg::Matrix mSmallerMatrix = osg::Matrix(
			vX_Smaller.x(), vX_Smaller.y(), vX_Smaller.z(), 0,
			vY_Smaller.x(), vY_Smaller.y(), vY_Smaller.z(), 0,
			vZ_Smaller.x(), vZ_Smaller.y(), vZ_Smaller.z(), 0,
			0, 0, 0, 1);
		// 在小层级空间坐标系下的方向
		osg::Vec3d vSmallerDir = mSmallerMatrix.preMult(m_vEyeFront);
		vSmallerDir.normalize();

		// 大层级空间
		SGMVector3 vGMX_Bigger, vGMY_Bigger, vGMZ_Bigger;
		GM_ENGINE.GetBiggerHierarchyCoord(vGMX_Bigger, vGMY_Bigger, vGMZ_Bigger);
		osg::Vec3d vX_Bigger = _GM2OSG(vGMX_Bigger);
		osg::Vec3d vY_Bigger = _GM2OSG(vGMY_Bigger);
		osg::Vec3d vZ_Bigger = _GM2OSG(vGMZ_Bigger);
		osg::Matrix mBiggerMatrix = osg::Matrix(
			vX_Bigger.x(), vX_Bigger.y(), vX_Bigger.z(), 0,
			vY_Bigger.x(), vY_Bigger.y(), vY_Bigger.z(), 0,
			vZ_Bigger.x(), vZ_Bigger.y(), vZ_Bigger.z(), 0,
			0, 0, 0, 1);
		// 在大层级空间坐标系下的方向
		osg::Vec3d vBiggerDir = mBiggerMatrix.preMult(m_vEyeFront);
		vBiggerDir.normalize();

		double fSmallerMix = pow(_Smoothstep(GM_MIN_RADIUS * 0.01, GM_MIN_RADIUS * 100, m_fCameraRadius), 0.4);
		double fBiggerMix = pow(_Smoothstep(GM_MAX_RADIUS * 0.01, GM_MAX_RADIUS * 100, m_fCameraRadius), 0.4);
		// 当前帧的前方向和上方向
		m_vEyeFront = _Mix(vSmallerDir, m_vEyeFront, fSmallerMix);
		m_vEyeFront = _Mix(m_vEyeFront, vBiggerDir, fBiggerMix);
		m_vEyeFront.normalize();
		m_vEyeUp = _Mix(vZ_Smaller, osg::Vec3d(0,0,1), fSmallerMix);
		m_vEyeUp = _Mix(m_vEyeUp, vZ_Bigger, fBiggerMix);
		m_vEyeUp.normalize();
	}
	break;
	default:
		break;
	}
	
	if (EGMM_DRIVE == m_eManipulator)
	{
		osg::Vec3d vEyeRightOld = m_vEyeFront ^ m_vEyeUp;
		vEyeRightOld.normalize();
		osg::Vec3d vEyeFrontOld = m_vEyeFront;
		vEyeFrontOld.normalize();
		osg::Vec3d vEyeUpOld = vEyeRightOld ^ vEyeFrontOld;
		vEyeUpOld.normalize();

		// 先平移
		m_sDrive.vSpeed += osg::Vec3d(	
			m_sDrive.fAccelerationRight - m_sDrive.fAccelerationLeft,
			m_sDrive.fAccelerationUp - m_sDrive.fAccelerationDown,
			m_sDrive.fAccelerationBack - m_sDrive.fAccelerationFront
		) * fDeltaTime;
		m_sDrive.vSpeed.x() = osg::clampBetween(m_sDrive.vSpeed.x(), -m_sDrive.vSpeed_Max.x(), m_sDrive.vSpeed_Max.x());
		m_sDrive.vSpeed.y() = osg::clampBetween(m_sDrive.vSpeed.y(), -m_sDrive.vSpeed_Max.y(), m_sDrive.vSpeed_Max.y());
		m_sDrive.vSpeed.z() = osg::clampBetween(m_sDrive.vSpeed.z(), -m_sDrive.vSpeed_Max.z(), m_sDrive.vSpeed_Max.z());
		if (m_sDrive.bBrakeMode)
		{
			m_sDrive.vSpeed *= 0.6;
			if(0.1 > m_sDrive.vSpeed.length())
			{
				m_sDrive.vSpeed = osg::Vec3d(0,0,0);
				m_sDrive.bBrakeMode = false;
			}
		}

		osg::Vec3d vHieMove = (m_sDrive.vSpeed / fUnit) * fDeltaTime;
		m_vEyeDeltaMove =
			vEyeRightOld * vHieMove.x()
			+ vEyeUpOld * vHieMove.y()
			- vEyeFrontOld * vHieMove.z();
		m_vTargetHierarchyPos += m_vEyeDeltaMove;
		m_vTranslate += m_vEyeDeltaMove;

		// 再旋转
		double fRollTarget = 0.0;
		if (m_sDrive.bRollRight || m_sDrive.bRollLeft)
		{
			if (m_sDrive.bRollRight)
			{
				fRollTarget += m_sDrive.fRoll_Max;
			}
			if (m_sDrive.bRollLeft)
			{
				fRollTarget -= m_sDrive.fRoll_Max;
			}
			m_sDrive.fRoll = _Mix(m_sDrive.fRoll, fRollTarget, 0.02);
		}
		else
		{
			m_sDrive.fRoll *= 0.8;
		}
		osg::Quat qRotate = osg::Quat(
			m_sDrive.fPitch * fDeltaTime, vEyeRightOld,
			m_sDrive.fYaw * fDeltaTime, vEyeUpOld,
			m_sDrive.fRoll * fDeltaTime, vEyeFrontOld);
		m_vEyeUp = qRotate * vEyeUpOld;
		m_vEyeUp.normalize();
		m_vEyeFront = qRotate * vEyeFrontOld;
		m_vEyeFront.normalize();

		if (!m_sDrive.bPitchOrYaw)
		{
			m_sDrive.fPitch *= 0.9;
			m_sDrive.fYaw *= 0.9;
		}

		// 更新旋转矩阵
		mCameraRotate.makeLookAt(osg::Vec3d(0, 0, 0), m_vEyeFront, m_vEyeUp);
		// 更新旋转姿态
		m_qRotate.set(mCameraRotate);
	}
	else
	{
		// 无论前面所有操作多么复杂，最终影响相机的只有旋转和平移
		// 所以观察模式下最终都只需要设置这两个变量
		
		// 上方向会根据眼点到目标点的距离变化
		mCameraRotate.makeLookAt(osg::Vec3d(0, 0, 0), m_vEyeFront, m_vEyeUp);
		// 更新旋转姿态
		m_qRotate.set(mCameraRotate);
		// 更新位置
		m_vTranslate = _GetTranslate();
	}
}

void CGMCameraManipulator::_SetManipulatorMode(const EGMC_MANIPULATOR eMode)
{
	m_eManipulator = eMode;

	switch (m_eManipulator)
	{
	case EGMM_WATCH:
	{
	}
	break;
	case EGMM_DRIVE:
	{
	}
	break;
	default:
		break;
	}

	m_vTerminalHierarchyPos = m_vTargetHierarchyPos;
	m_vOriginHierarchyPos = m_vTargetHierarchyPos;
}

void CGMCameraManipulator::_SetWatchState(const EGMC_WATCH eState)
{
	m_eWatchState = eState;

	osg::Vec3d vPos = osg::Vec3d(0, 0, 0);
	m_fOriginPitch = asin(m_vEyeFront.z());
	m_fOriginYaw = atan2(m_vEyeFront.x(), m_vEyeFront.y());
	m_fCameraStartRadius = m_fCameraRadius;

	// 在“EGMC_WATCH_HOME”过程中，相机yaw不变，pitch、目标点位置、焦距 随指数函数变化
	// 在“EGMC_WATCH_GOTO”过程中，相机yaw，pitch、目标点位置、焦距 随指数函数变化
	int iHie = GM_ENGINE.GetHierarchy();
	switch (eState)
	{
	case (EGMC_WATCH_FREE):
	{
		vPos = m_vTerminalHierarchyPos;
	}
	break;
	case (EGMC_WATCH_ZOOM):
	{
		if (4 == iHie)
		{
			vPos = m_vLastMouseHierarchyPos;
			// 让中心点始终在银盘上，不至于点到银河系之外的真空
			double fLenR = vPos.length() / (GM_ENGINE.GetGalaxyRadius()/GM_ENGINE.GetUnit());
			if (fLenR > 1.0) vPos /= fLenR;
		}
		else if (4 < iHie)
		{
			// 大于第4层级空间，让中心点始终在(0,0,0)
			vPos = osg::Vec3d(0,0,0);
		}
		else if (3 == iHie)
		{
			vPos = m_vLastMouseHierarchyPos;
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
	case (EGMC_WATCH_MOVE):
	{
		m_fYawSpeed = 0;
		vPos = m_vTargetHierarchyPos;
		m_fTerminalPitch = m_fOriginPitch;
		m_fTerminalYaw = m_fOriginYaw;
	}
	break;
	case (EGMC_WATCH_GOTO):
	{
		/* 由于本产品的特殊性，需要将银河系中心作为宇宙的中心
		* 这样就不用考虑用double表示恒星在6级空间中的位置的误差问题 */
		m_vStarWorldPos = _GM2OSG(GM_ENGINE.GetCurrentStarWorldPos());

		switch (iHie)
		{
		case 0:
		{
		}
		break;
		case 1:
		{
		}
		break;
		case 2:
		{
			vPos = osg::Vec3d(0, 0, 0);
		}
		break;
		case 3:
		{
			// 将选中的行星或者恒星作为目标点
			vPos = m_vPlanetSolarPos / GM_ENGINE.GetUnit();
		}
		break;
		case 4:
		{
			vPos = m_vStarWorldPos / GM_ENGINE.GetUnit();
		}
		break;
		case 5:
		{
		}
		break;
		case 6:
		{
		}
		break;
		default:
			break;
		}

		osg::Vec3d vGo = vPos - m_vTranslate;
		double fDistance = vGo.length();

		m_fYawSpeed = 0;
		m_fTerminalPitch = GM_GOTO_PITCH;
		m_fTerminalYaw = atan2(vGo.x(), vGo.y());

		m_fJumpDuration = GM_GOTO_DURATION * std::fmax(0.2, 1.0 - exp(-fDistance / GM_HOME_RADIUS));
		_SetMix(2 * m_fJumpDuration);
	}
	break;
	case (EGMC_WATCH_JUMP):
	case (EGMC_WATCH_HOME):
	{
		m_fYawSpeed = 0;
		m_fTerminalPitch = GM_HOME_PITCH;
		m_fTerminalYaw = m_fOriginYaw;

		switch (iHie)
		{
		case 0:
		{
		}
		break;
		case 1:
		{
		}
		break;
		case 2:
		{
			m_fJumpDuration = GM_HOME_DURATION;
			_SetMix(2.5 * m_fJumpDuration);
		}
		break;
		case 3:
		{
			osg::Vec3d vStarHiePos4 = m_vStarWorldPos / GM_ENGINE.GetUnit(4);
			osg::Vec3d vEyeHiePos4 = vStarHiePos4 + m_vTranslate / GM_UNIT_SCALE;
			osg::Vec3d vHomeEyePos = -m_vEyeFront * GM_HOME_RADIUS;
			double fDistance = (vEyeHiePos4 - vHomeEyePos).length();

			m_fJumpDuration = max(0.1, GM_HOME_DURATION * (1 - exp(-fDistance / GM_HOME_RADIUS)));
			_SetMix(3 * m_fJumpDuration);
		}
		break;
		case 4:
		{
			osg::Vec3d vEyeHiePos4 = m_vTranslate;
			osg::Vec3d vHomeEyePos = -m_vEyeFront * GM_HOME_RADIUS;
			double fDistance = (vEyeHiePos4 - vHomeEyePos).length();

			m_fJumpDuration = max(0.1, GM_HOME_DURATION * (1 - exp(-fDistance / GM_HOME_RADIUS)));
			_SetMix(3 * m_fJumpDuration);
		}
		break;
		case 5:
		{
		}
		break;
		case 6:
		{
		}
		break;
		default:
			break;
		}
	}
	break;
	case (EGMC_WATCH_ULTIMATE):
	{
		m_fJumpDuration = 30.0;
		_SetMix(1e-6 * m_fJumpDuration);

		m_fYawSpeed = 0;
		m_fTerminalPitch = GM_HOME_PITCH;
		m_fTerminalYaw = m_fOriginYaw;

		vPos = m_vTargetHierarchyPos;
	}
	break;
	default:
		break;
	}

	m_fTimeJumpStart = osg::Timer::instance()->time_s();
	m_fTimeSinceJump = 0.0;
	m_vOriginHierarchyPos = m_vTargetHierarchyPos;
	m_vTerminalHierarchyPos = vPos;
}

void CGMCameraManipulator::_SetPitchYaw(const float fXnormalized, const float fYnormalized)
{
	m_sDrive.fPitch = 2 * osg::sign(fYnormalized) * fYnormalized * fYnormalized;
	m_sDrive.fYaw = -2 * osg::sign(fXnormalized) * fXnormalized * fXnormalized;

	m_sDrive.bPitchOrYaw = (abs(m_sDrive.fPitch)>0) || (abs(m_sDrive.fYaw) > 0);
}

double CGMCameraManipulator::_GetMix(const double fT, const EGMC_WATCH eState)
{
	double fMix = 0.0;
	switch (eState)
	{
	case EGMC_WATCH_FREE:
	case EGMC_WATCH_ZOOM:
	case EGMC_WATCH_MOVE:
	{
	}
	break;
	case EGMC_WATCH_GOTO:
	case EGMC_WATCH_ULTIMATE:
	{
		// 半径、角度的混合系数
		fMix = (1 + 2*exp(-fT*3*m_vMix.x()) - 3*exp(-fT*2*m_vMix.x())) / m_vMix.y();
		fMix = osg::clampBetween(fMix, 0.0, 1.0);
	}
	break;
	case EGMC_WATCH_HOME:
	case EGMC_WATCH_JUMP:
	{
		double fTN = 1 - fT;
		fMix = (1 + 2*exp(-fTN*3*m_vMix.x()) - 3*exp(-fTN*2*m_vMix.x())) / m_vMix.y();
		fMix = osg::clampBetween(1-fMix, 0.0, 1.0);
	}
	break;
	default:
		break;
	}
	return fMix;
}

void CGMCameraManipulator::_SetMix(const double fScaleTime)
{
	m_vMix.x() = fScaleTime;
	m_vMix.y() = 1 + 2*exp(-3*m_vMix.x()) - 3*exp(-2*m_vMix.x());
}

osg::Vec3d CGMCameraManipulator::_GetTranslate() const
{
	return m_vTargetHierarchyPos - m_vEyeFront * m_fCameraRadius;
}

bool CGMCameraManipulator::_IsTargetInVacuum(const int iHie) const
{
	bool bVacuumTarget = false;
	switch (iHie)
	{
	case 0:
	{

	}
	break;
	case 1:
	{
	}
	break;
	case 2:
	{
		// 目标点离超新星或者行星太远，就认为是真空
		double fUnit = GM_ENGINE.GetUnit();
		osg::Vec3d vPlanetHiePos = osg::Vec3d(0, 0, 0) / fUnit;
		double fHieCelestialRadius = GM_ENGINE.GetCelestialMeanRadius() / fUnit;
		//暂定大于1.5倍天体半径就是远
		bVacuumTarget = (m_vTerminalHierarchyPos - vPlanetHiePos).length() > (1.5*fHieCelestialRadius);
		if (m_bTarget2CelestialBody) bVacuumTarget = false;
	}
	break;
	case 3:
	{
		// 目标点离超新星（中心点）太远，就认为是真空
		bVacuumTarget = (m_vTerminalHierarchyPos.length() > 0.01);
		if (m_bTarget2CelestialBody) bVacuumTarget = false;
	}
	break;
	case 4:
	{
		// 目标点离超新星太远，就认为是真空
		osg::Vec3d vStarHiePos = m_vStarWorldPos / GM_ENGINE.GetUnit();
		bVacuumTarget = (m_vTerminalHierarchyPos - vStarHiePos).length() > GM_MIN_RADIUS*0.2;
	}
	break;
	case 5:
	{
	}
	break;
	case 6:
	{
	}
	break;
	default:{}
	}
	
	return bVacuumTarget;
}

bool CGMCameraManipulator::_IsIntoGround(osg::Vec3d& vHiePos) const
{
	if (2 < GM_ENGINE.GetHierarchy()) return false;

	osg::Vec3d vOutDir = vHiePos;
	double fPointR = vOutDir.normalize();
	osg::Vec3d vEyeHiePos = m_vTranslate;
	osg::Vec3d vEyeOutDir = vEyeHiePos;
	// 眼点到世界中心点的距离
	double fEyeR = vEyeOutDir.normalize();

	osg::Vec3d vEye2Term = vHiePos - vEyeHiePos;
	osg::Vec3d vEye2TermDir = vEye2Term;
	// 该点到眼点的距离
	double fPoint2EyeDis = vEye2TermDir.normalize();
	// 两条射线的夹角余弦值
	double fCosA = (fEyeR * fEyeR + fPoint2EyeDis * fPoint2EyeDis - fPointR * fPointR) / (2 * fEyeR * fPoint2EyeDis);
	double fSinA = sqrt(max(0, 1 - fCosA * fCosA));
	// 该射线与球面最近点与世界中心点的距离
	double fRootR = fSinA * fEyeR;

	osg::Vec3d vCelesNorth = _GM2OSG(GM_ENGINE.GetCelestialNorth());
	double fLatitude = osg::RadiansToDegrees(osg::PI_2 - acos(vCelesNorth * vOutDir));
	double fCelesR = GM_ENGINE.GetCelestialRadius(fLatitude) / GM_ENGINE.GetUnit();
	// 将目标点定在天体表面，避免穿地
	if (fPointR < 1e-9)
	{
		// 如果最终目标的坐标与原点很接近，就认为在原点
		vHiePos = vEyeOutDir * fCelesR;
		return true;
	}
	else if (vOutDir == vEyeOutDir)
	{
		// 如果最终目标、原点、眼点 共线
		vHiePos = vOutDir * fCelesR;
		return true;
	}
	else if (fPointR <= fCelesR || (fPoint2EyeDis > fEyeR && fRootR < fCelesR))
	{
		double fSinD = min(1, fSinA * fEyeR / fCelesR); // fSinD <= 1
		double fCosD = sqrt(1 - fSinD * fSinD);
		vHiePos = vEyeHiePos + vEye2TermDir * (fEyeR * fCosA - fCelesR * fCosD);
		return true;
	}
	else{}

	return false;
}
