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

using namespace GM;

/*************************************************************************
 Macro Defines
*************************************************************************/

#define GM_MAX_PITCH				(0.999)
#define GM_HOME_PITCH				(osg::PI*0.14)
#define GM_GOTO_PITCH				(osg::PI*0.48)
#define GM_HOME_RADIUS				(12.0)

/*************************************************************************
CGMCameraManipulator Methods
*************************************************************************/

CGMCameraManipulator::CGMCameraManipulator() : osgGA::CameraManipulator(),
	m_vTranslate(osg::Vec3d(0, -9, 9)), m_qRotate(osg::Quat(-osg::PI_4, osg::Vec3d(1, 0, 0), 0, osg::Vec3d(0, 1, 0), 0, osg::Vec3d(0, 0, 1))),
	m_fDeltaStep(0.0f), m_fConstantStep(0.5f), 
	m_fSignYaw(0.0f), m_fTimeLastFrame(0.0), m_fTimeJumpStart(0.0), m_fTimePushFrame(0.0),
	m_fCameraRadius(12.0), m_fRadiusAcceleration(0.0), m_fRadiusWeakRatio(3),
	m_fConstSpinSpeed(osg::inDegrees(0.6)), m_fSpinSpeed(osg::inDegrees(0.0)),
	m_fPitchSpeed(0.0), m_fTerminalPitch(GM_GOTO_PITCH), m_fOriginPitch(GM_HOME_PITCH),
	m_fSpinAcceleration(0.0), m_fSpinWeakRatio(3),m_fTargetMix(0.0),
	m_vTargetWorldPos(osg::Vec3d(0, 0, 0)),
	m_vTerminalWorldPos(osg::Vec3d(0, 0, 0)), m_vOriginWorldPos(osg::Vec3d(0, 0, 0)),
	m_vStarWorldPos(osg::Vec3d(0, 0, 0)),
	m_vMouseWorldPos(osg::Vec3d(0, 0, 0)), m_vLastMouseWorldPos(osg::Vec3d(0, 0, 0)), 
	m_vMousePushPos(osg::Vec2f(0.0f, 0.0f)), m_vMouseLastPos(osg::Vec2f(0.0f,0.0f)),
	m_eFlyMode(EGMA_FLY_FREE), m_bEdit(false)
{
	osg::Vec3d vDirection = osg::Vec3d(0.0, -std::cos(m_fOriginPitch),std::sin(m_fOriginPitch));
	m_vTranslate = _GetTranslate(vDirection);
	m_qRotate = osg::Quat(
		m_fOriginPitch - osg::PI_2, osg::Vec3d(1.0, 0.0, 0.0),
		0, osg::Vec3d(0.0, 1.0, 0.0),
		0, osg::Vec3d(0.0, 0.0, 1.0)
	);
}

bool
CGMCameraManipulator::screenToWorld(float x, float y, osg::View* theView, osg::Vec3d& vWorldPos) const
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
	vWorldPos = osg::Vec3d(
		startVertex.x() + fRatio * (endVertex.x() - startVertex.x()),
		startVertex.y() + fRatio * (endVertex.y() - startVertex.y()),
		0);
	return true;
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

		osg::Vec3d vCurrTargetWorldPos = m_vOriginWorldPos * (1.0 - m_fTargetMix) + m_vTerminalWorldPos * m_fTargetMix;

		double fDeltaAngle = 0.0;
		if (vCurrTargetWorldPos != m_vTargetWorldPos)
		{
			if (EGMA_FLY_HOME != m_eFlyMode)
			{
				osg::Vec3d vCurrTargetViewPos = vCurrTargetWorldPos - m_vTranslate;
				osg::Vec2d vCurrTargetViewDir2 = osg::Vec2d(vCurrTargetViewPos.x(), vCurrTargetViewPos.y());
				vCurrTargetViewDir2.normalize();
				osg::Vec3d vLastTargetViewPos = m_vTargetWorldPos - m_vTranslate;
				osg::Vec2d vLastTargetViewDir2 = osg::Vec2d(vLastTargetViewPos.x(), vLastTargetViewPos.y());
				vLastTargetViewDir2.normalize();
				//这里acos内部的值必须在作用域内
				fDeltaAngle = m_fSignYaw *
					std::acos(osg::clampBetween(vCurrTargetViewDir2 * vLastTargetViewDir2, 0.0, 1.0));
			}
			m_vTargetWorldPos = vCurrTargetWorldPos;
		}

		double accelerationFact = std::sqrt(max(1e-6,(GM_HOME_RADIUS - m_fCameraRadius) / GM_HOME_RADIUS));
		double fFlyingTime = 0.0;
		switch (m_eFlyMode)
		{
		case EGMA_FLY_FREE:
		{
			fFlyingTime = timeCurrFrame - m_fTimePushFrame; //单位:秒
		}
		break;
		case EGMA_FLY_CONTROL:
		{
			fFlyingTime = timeCurrFrame - m_fTimePushFrame; //单位:秒
			float fDeltaY = ea.getY() - m_vMousePushPos.y();
			float fSignY = osg::sign(fDeltaY);
			m_fRadiusAcceleration = m_fCameraRadius * fSignY * fDeltaTime * min(1.8, abs(fDeltaY) * 0.003);
		}
		break;
		case EGMA_FLY_GOTO:
		{
			fFlyingTime = timeCurrFrame - m_fTimeJumpStart; //单位:秒
			m_fRadiusAcceleration = -m_fCameraRadius * 0.026 * accelerationFact;
			osg::Vec3d vDeltaPos = vCurrTargetWorldPos - m_vTerminalWorldPos;
			double fDistance = vDeltaPos.length();
			if (fDistance < 0.001 || m_fCameraRadius < 0.05)
			{
 				m_eFlyMode = EGMA_FLY_FREE;
			}
		}
		break;
		case EGMA_FLY_HOME:
		{
			fFlyingTime = timeCurrFrame - m_fTimeJumpStart; //单位:秒
			m_fRadiusAcceleration = m_fCameraRadius * 0.06 * accelerationFact;
			if (m_fCameraRadius >= GM_HOME_RADIUS)
			{
				_StartToJump(EGMA_FLY_GOTO);
			}
		}
		break;
		}

		// 由跳跃飞行引起的俯仰角变化
		double fDeltaPitch = 0.0;

		if ((m_fRadiusAcceleration < 0 || EGMA_FLY_HOME == m_eFlyMode) && m_fTargetMix < 1.0)
		{
			double fTargetMixTimeScale = 1.5;		// 越大，m_fTargetMix 变化的越快
			switch (m_eFlyMode)
			{
			case (EGMA_FLY_GOTO):
			{
				fTargetMixTimeScale = 1.8;
			}
			break;
			case (EGMA_FLY_HOME):
			{
				fTargetMixTimeScale = 0.2;
			}
			break;
			}
			// 借用这个差值俯仰角，记录上一帧的俯仰角
			double fLastTargetMix = m_fTargetMix;
			// 如果没有最小值限制，这个公式在0到正无穷的区间上积分 == 1
			// 但为了保证最后不会移动太慢，设置了最小增量，所以必须加一个1.0的限制
			m_fTargetMix += fDeltaTime * 
				max(0.0005, fTargetMixTimeScale * 2 *
				exp(-fFlyingTime * fTargetMixTimeScale) * (1.0 - exp(-fFlyingTime * fTargetMixTimeScale)));
			m_fTargetMix = min(1.0, m_fTargetMix);

			if (EGMA_FLY_HOME == m_eFlyMode || EGMA_FLY_GOTO == m_eFlyMode)
			{
				// 计算差值俯仰角
				fDeltaPitch = (m_fTargetMix - fLastTargetMix)*(m_fTerminalPitch - m_fOriginPitch);
			}
		}

		if (screenToWorld(ea.getX(), ea.getY(), aa.asView(), m_vMouseWorldPos))
		{
			GM_ENGINE_PTR->SetMousePosition(m_vMouseWorldPos);
		}

		m_fCameraRadius = std::fmax(0.001, m_fCameraRadius + m_fRadiusAcceleration);
		m_fRadiusAcceleration *= exp2(-fDeltaTime*m_fRadiusWeakRatio);
		m_fSpinSpeed = osg::clampBetween(m_fSpinSpeed+m_fSpinAcceleration, -6.0, 6.0);
		m_fSpinSpeed *= exp2(-fDeltaTime * m_fSpinWeakRatio);
		osg::Matrix mRotateHorizontal = osg::Matrix::rotate(
			fDeltaAngle + (m_fSpinSpeed + m_fConstSpinSpeed) * fDeltaTime,
			osg::Vec3d(0, 0, 1));

		// 修改姿态
		osg::Matrix mCameraRotate;
		m_qRotate.get(mCameraRotate);
		osg::Vec3d vEye, vCenter, vUp; //相机的相对姿态
		mCameraRotate.getLookAt(vEye, vCenter, vUp);//这里的vEye不是实际相机的位置，而是0，0，0
		osg::Vec3d vDirection  = vCenter - vEye; //相机视点方向
		osg::Vec3d vRight = vDirection ^ vUp;
		vRight.normalize();

		osg::Matrix mRotateVertical = osg::Matrix::identity();
		// 判断俯仰角是否超过界限
		if (EGMA_FLY_HOME == m_eFlyMode || EGMA_FLY_GOTO == m_eFlyMode)
		{
			mRotateVertical = osg::Matrix::rotate(fDeltaPitch, vRight);
		}
		else if ((vDirection.z() < GM_MAX_PITCH) && (m_fPitchSpeed > 0)
			|| (vDirection.z() > -GM_MAX_PITCH) && (m_fPitchSpeed < 0))
		{
			mRotateVertical = osg::Matrix::rotate(m_fPitchSpeed * fDeltaTime, vRight);
		}

		vDirection = vDirection * mRotateHorizontal * mRotateVertical;
		vDirection.normalize();
		mCameraRotate.makeLookAt(osg::Vec3d(0, 0, 0), vDirection, osg::Vec3d(0, 0, 1));
		m_qRotate.set(mCameraRotate);

		// 修改位置
		m_vTranslate = _GetTranslate(vDirection);

		m_fPitchSpeed *= exp2(-fDeltaTime * 9.7);
		m_fSpinAcceleration *= exp2(-fDeltaTime * 7.5);
		m_fTimeLastFrame = timeCurrFrame;
	}
	break;
	case (osgGA::GUIEventAdapter::PUSH):
	{
		m_fTimePushFrame = osg::Timer::instance()->time_s();
		m_vLastMouseWorldPos = m_vMouseWorldPos;
		m_vMousePushPos = osg::Vec2f(ea.getX(), ea.getY());
		m_vMouseLastPos = m_vMousePushPos;

		if (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON == ea.getButton())
		{
			m_eFlyMode = EGMA_FLY_CONTROL;
			m_fSignYaw = -osg::sign(ea.getXnormalized());
			m_fTargetMix = 0.0;
			m_vOriginWorldPos = m_vTargetWorldPos;
			m_vTerminalWorldPos = m_vMouseWorldPos;
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
			m_eFlyMode = EGMA_FLY_FREE;
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
			if (m_bEdit)
			{
				GM_ENGINE_PTR->SetCapture(true);
			}
		}
		else if (osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON == ea.getButtonMask())
		{
			double fAngularVelocity = _GetAngularVelocity(m_vMouseWorldPos, m_vLastMouseWorldPos);
			float fDeltaX = ea.getX() - m_vMouseLastPos.x();
			float fDeltaY = ea.getY() - m_vMouseLastPos.y();
			if (1.0f < std::abs(fDeltaY / fDeltaX)) // 控制俯仰
			{
				m_fPitchSpeed = (m_fPitchSpeed + fDeltaY * 0.09) * 0.5;
			}
			else // 控制自传
			{
				m_fSpinAcceleration = 6*fAngularVelocity;
			}
		}

		m_vLastMouseWorldPos = m_vMouseWorldPos;
		m_vMouseLastPos = osg::Vec2f(ea.getX(), ea.getY());
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
			m_eFlyMode = EGMA_FLY_FREE;
			m_vLastMouseWorldPos = osg::Vec3d(0, 0, 0);
			m_vMouseWorldPos = osg::Vec3d(0, 0, 0);
			m_vOriginWorldPos = osg::Vec3d(0, 0, 0);
			m_vTerminalWorldPos = osg::Vec3d(0, 0, 0);
			m_vTargetWorldPos = osg::Vec3d(0, 0, 0);
			m_fRadiusAcceleration = 0.0;
			m_fCameraRadius = 12.0;
			m_fPitchSpeed = 0;
			m_fSpinAcceleration = 0;
			m_fSpinSpeed = 0;
			m_qRotate = osg::Quat(
				GM_HOME_PITCH - osg::PI_2, osg::Vec3d(1.0, 0.0, 0.0),
				0, osg::Vec3d(0.0, 1.0, 0.0),
				0, osg::Vec3d(0.0, 0.0, 1.0)
			);
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
		case osgGA::GUIEventAdapter::KEY_1:
		{
			GM_ENGINE_PTR->Play();
		}
		break;
		case osgGA::GUIEventAdapter::KEY_2:
		{
			GM_ENGINE_PTR->Pause();
		}
		break;
		case osgGA::GUIEventAdapter::KEY_3:
		{
			GM_ENGINE_PTR->Stop();
		}
		break;
		case osgGA::GUIEventAdapter::KEY_4:
		{
			GM_ENGINE_PTR->Next();
			if (m_fCameraRadius >= GM_HOME_RADIUS)
			{
				_StartToJump(EGMA_FLY_GOTO);
			}
			else
			{
				_StartToJump(EGMA_FLY_HOME);
			}
		}
		break;
		case 19:
		{
			// ctrl + s
			GM_ENGINE_PTR->Save();
		}
		break;
		}
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
	}
	break;
	case (osgGA::GUIEventAdapter::DOUBLECLICK):
	{
		if (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON == ea.getButtonMask())
		{
			GM_ENGINE_PTR->SetAudio(m_vMouseWorldPos.x(), m_vMouseWorldPos.y());
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

		if (m_fCameraRadius >= GM_HOME_RADIUS)
		{
			_StartToJump(EGMA_FLY_GOTO);
		}
		else
		{
			_StartToJump(EGMA_FLY_HOME);
		}
	}
}

void
CGMCameraManipulator::_StartToJump(EGMC_FLY eMode)
{
	osg::Vec3d vPos = osg::Vec3d(0, 0, 0);
	osg::Matrix mCameraRotate;
	m_qRotate.get(mCameraRotate);
	osg::Vec3d vEye, vCenter, vUp; //相机的相对姿态
	mCameraRotate.getLookAt(vEye, vCenter, vUp);//这里的vEye不是实际相机的位置，而是0，0，0
	osg::Vec3d vDirection = vCenter - vEye; //相机视点方向
	vDirection.normalize();
	m_fOriginPitch = asin(vDirection.z()) + osg::PI_2;

	switch (eMode)
	{
	case (EGMA_FLY_GOTO):
	{
		m_eFlyMode = eMode;
		m_vStarWorldPos = GM_ENGINE_PTR->GetCurrentStarWorldPos();
		vPos = m_vStarWorldPos;
		m_fTerminalPitch = GM_GOTO_PITCH;
	}
	break;
	case (EGMA_FLY_HOME):
	{
		m_eFlyMode = eMode;
		m_fTerminalPitch = GM_HOME_PITCH;
	}
	break;
	default:
	return;
	}

	osg::Vec3d vCurrTargetViewPos = vPos - (osg::Vec3d(0,0,0) - vDirection * m_fCameraRadius);
	osg::Vec3d vCurrTargetViewDir = vCurrTargetViewPos;
	vCurrTargetViewDir.normalize();

	m_fSignYaw = 1;
	double cosV = vDirection * vCurrTargetViewDir;
	if (cosV < 1.0 && cosV > -1.0)
	{
		// 两个向量不共面，可以X乘
		osg::Vec3d vCross = vDirection ^ vCurrTargetViewDir;
		m_fSignYaw = osg::sign(vCross.z());
	}

	m_fTimeJumpStart = osg::Timer::instance()->time_s();
	m_fTargetMix = 0.0;
	m_vOriginWorldPos = m_vTargetWorldPos;
	m_vTerminalWorldPos = vPos;
}

double
CGMCameraManipulator::_Remap(const double fX,
	const double fMin, const double fMax,
	const double fMinNew, const double fMaxNew)
{
	double fY = (osg::clampBetween(fX, fMin, fMax) - fMin) / (fMax - fMin);
	return fMinNew + fY * (fMaxNew - fMinNew);
}

osg::Vec3d
CGMCameraManipulator::_GetTranslate(const osg::Vec3d vDirection)
{
	return m_vTargetWorldPos - vDirection * m_fCameraRadius;
}

double
CGMCameraManipulator::_GetAngularVelocity(const osg::Vec3d vPos, const osg::Vec3d vLastPos)
{
	osg::Vec3d vSpeed = vPos - vLastPos;
	osg::Vec3d vWorldPickDir = vPos - m_vTargetWorldPos;
	double fPickLength = vWorldPickDir.normalize();
	osg::Vec3d vWorldPickTangent = vWorldPickDir ^ osg::Vec3d(0, 0, 1);
	vWorldPickTangent.normalize();
	return vWorldPickTangent * vSpeed / fPickLength;
}
