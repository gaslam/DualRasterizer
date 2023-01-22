#pragma once
#include "Effect.h"
class EffectPosTransp : public Effect
{
public:
	EffectPosTransp(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~EffectPosTransp() override;

	EffectPosTransp(const EffectPosTransp& other) = delete;
	EffectPosTransp(EffectPosTransp&& other) = delete;
	EffectPosTransp& operator=(const EffectPosTransp& other) = delete;
	EffectPosTransp& operator=(EffectPosTransp&& other) = delete;


private:

	void CreateInputLayout() override;

};