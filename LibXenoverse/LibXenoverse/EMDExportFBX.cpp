
//#include "fbxsdk/core/arch/fbxtypes.h"
#include <fstream>

namespace LibXenoverse
{


#ifdef LIBXENOVERSE_FBX_SUPPORT








/*-------------------------------------------------------------------------------\
|                             exportFBX											 |
\-------------------------------------------------------------------------------*/
void EMD::exportFBX(FbxScene *scene, std::vector<ESK::FbxBonesInstance_DBxv> &global_fbx_bones, std::vector<EMB*> listTexturePackEMB, EMM* emmMaterial, bool wantNoTexture)
{
	FbxNode *node = FbxNode::Create(scene, name.c_str());
	scene->GetRootNode()->AddChild(node);


	node->LclTranslation.Set(FbxVector4(0, 0, 0));
	node->LclRotation.Set(FbxVector4(0, 0, 0, 1));
	node->LclScaling.Set(FbxVector4(1, 1, 1));


	size_t nbModels = models.size();
	for (size_t i = 0; i < nbModels; i++)
		models.at(i)->exportFBX(scene, global_fbx_bones, node, i, listTexturePackEMB, emmMaterial, wantNoTexture);
}
/*-------------------------------------------------------------------------------\
|                             exportFBX											 |
\-------------------------------------------------------------------------------*/
void EMDModel::exportFBX(FbxScene *scene, std::vector<ESK::FbxBonesInstance_DBxv> &global_fbx_bones, FbxNode* parentNode, size_t indexModel, std::vector<EMB*> listTexturePackEMB, EMM* emmMaterial, bool wantNoTexture)
{
	FbxNode *node = FbxNode::Create(scene, name.c_str());
	//FbxNode *node = FbxNode::Create(scene, (name +"_test").c_str());			//test todo remove
	node->LclTranslation.Set(FbxVector4(0, 0, 0));
	node->LclRotation.Set(FbxVector4(0, 0, 0));
	node->LclScaling.Set(FbxVector4(1, 1, 1));

	parentNode->AddChild(node);

	
	//Test Todo remove. to have Node animation (not just on bones).
	FbxNode* forceBoneNode = 0;
	if (false)
	{
		const int lNodeCount = scene->GetSrcObjectCount<FbxNode>();
		for (int lIndex = 0; lIndex < lNodeCount; lIndex++)
		{
			FbxNode* fbxNode = scene->GetSrcObject<FbxNode>(lIndex);
			if ((!fbxNode) || (!fbxNode->GetSkeleton()) || (string(fbxNode->GetName()) != name))	//in this case the bones have the same name of a EmdModel
				continue;

			forceBoneNode = fbxNode;
			break;
		}
	}

	size_t nbMesh = meshes.size();
	for (size_t i = 0; i < nbMesh; i++)
		meshes.at(i)->exportFBX(scene, global_fbx_bones, node, listTexturePackEMB, emmMaterial, wantNoTexture, forceBoneNode);
}
/*-------------------------------------------------------------------------------\
|                             exportFBX											 |
\-------------------------------------------------------------------------------*/
void EMDMesh::exportFBX(FbxScene *scene, std::vector<ESK::FbxBonesInstance_DBxv> &global_fbx_bones, FbxNode* parentNode, std::vector<EMB*> listTexturePackEMB, EMM* emmMaterial, bool wantNoTexture, FbxNode* forceBoneNode)
{
	FbxNode *node = FbxNode::Create(scene, name.c_str());
	//FbxNode *node = FbxNode::Create(scene, (name + "_test").c_str());			//test todo remove
	node->LclTranslation.Set(FbxVector4(0, 0, 0));
	node->LclRotation.Set(FbxVector4(0, 0, 0));
	node->LclScaling.Set(FbxVector4(1, 1, 1));

	parentNode->AddChild(node);

	size_t nbSubMesh = submeshes.size();
	for (size_t i = 0; i < nbSubMesh; i++)
		submeshes.at(i)->exportFBX(scene, global_fbx_bones, node, listTexturePackEMB, emmMaterial, wantNoTexture, forceBoneNode);
}
/*-------------------------------------------------------------------------------\
|                             EMDSubmesh										 |
\-------------------------------------------------------------------------------*/
void EMDSubmesh::exportFBX(FbxScene *scene, std::vector<ESK::FbxBonesInstance_DBxv> &global_fbx_bones, FbxNode* parentNode, std::vector<EMB*> listTexturePackEMB, EMM* emmMaterial, bool wantNoTexture, FbxNode* forceBoneNode)
{
	FbxNode *node = FbxNode::Create(scene, name.c_str());
	node->LclTranslation.Set(FbxVector4(0, 0, 0));
	node->LclRotation.Set(FbxVector4(0, 0, 0));
	node->LclScaling.Set(FbxVector4(1, 1, 1));

	if (!forceBoneNode)
	{
		parentNode->AddChild(node);
	}else {
		forceBoneNode->AddChild(node);
		forceBoneNode = 0;
	}


	FbxMesh* fbxMesh = FbxMesh::Create(scene, name.c_str());
	if (!fbxMesh)
		return;
	node->SetNodeAttribute(fbxMesh);					//attach mesh to node.
	


	// Materials part
	FbxGeometryElementMaterial* lMaterialElement = fbxMesh->CreateElementMaterial();			//add a material use.
	lMaterialElement->SetMappingMode(FbxGeometryElement::eByPolygon);
	lMaterialElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);
	FbxSurfaceMaterial*	fxbMaterial = exportFBXMaterial(scene, name, listTexturePackEMB, emmMaterial, wantNoTexture);	//in submesh , name is also materialName
	node->AddMaterial(fxbMaterial);




	fbxMesh->InitControlPoints( vertices.size() );		//resize by the numbers of Vertex.



	//Add declaration of Vertex format.
	uint16_t flags = vertex_type_flag;
		
	//(flags & EMD_VTX_FLAG_POS)						//position is always into Fbx mesh declaration. no need to do anythings.
	FbxVector4* fbxControlPointsPosition = fbxMesh->GetControlPoints();						//pointer of array , for position only


	FbxGeometryElementNormal* fbxGeometryElementNormal = NULL;
	if (flags & EMD_VTX_FLAG_NORM)
	{
		fbxGeometryElementNormal = fbxMesh->CreateElementNormal();
		fbxGeometryElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);
		fbxGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eDirect);
	}

	FbxGeometryElementTangent* fbxGeometryElementTangent = NULL;
	if (flags & EMD_VTX_FLAG_TANGENT)
	{
		fbxGeometryElementTangent = fbxMesh->CreateElementTangent();
		fbxGeometryElementTangent->SetMappingMode(FbxGeometryElement::eByControlPoint);
		fbxGeometryElementTangent->SetReferenceMode(FbxGeometryElement::eDirect);
	}

	FbxGeometryElementVertexColor* fbxGeometryElementColor = NULL;
	if (flags & EMD_VTX_FLAG_COLOR)
	{
		fbxGeometryElementColor = fbxMesh->CreateElementVertexColor();
		fbxGeometryElementColor->SetMappingMode(FbxGeometryElement::eByControlPoint);
		fbxGeometryElementColor->SetReferenceMode(FbxGeometryElement::eDirect);
	}
	
	FbxGeometryElementUV* fbxGeometryElementUV = NULL;
	if (flags & EMD_VTX_FLAG_TEX)
	{
		fbxGeometryElementUV = fbxMesh->CreateElementUV("UV");
		fbxGeometryElementUV->SetMappingMode(FbxGeometryElement::eByControlPoint);
		fbxGeometryElementUV->SetReferenceMode(FbxGeometryElement::eDirect);
	}

	FbxGeometryElementUV* fbxGeometryElementUV2 = NULL;
	if (flags & EMD_VTX_FLAG_TEX2)
	{
		fbxGeometryElementUV2 = fbxMesh->CreateElementUV("UV2");
		fbxGeometryElementUV2->SetMappingMode(FbxGeometryElement::eByControlPoint);
		fbxGeometryElementUV2->SetReferenceMode(FbxGeometryElement::eDirect);
	}

	


	if (forceBoneNode)
		int aa = 42;

	
	/*
	if (((flags & EMD_VTX_FLAG_BLEND_WEIGHT) == 0) && (forceBoneNode))				//if it use bones influence. case force to match
		flags = flags | EMD_VTX_FLAG_BLEND_WEIGHT;
	else
		forceBoneNode = false;														//if allready have blend, stop force.
	*/
	
	FbxSkin* lSkin = NULL;
	if (flags & EMD_VTX_FLAG_BLEND_WEIGHT)				//if it use bones influence.
	{
		lSkin = FbxSkin::Create(scene, ("Skin_"+ name).c_str());
		if (lSkin != NULL)
			fbxMesh->AddDeformer(lSkin);													//use Skin on mesh
	}

	



	//Export Vertices.
	float r = 0;
	float g = 0;
	float b = 0;
	float a = 0;
	size_t nbVertex = vertices.size();
	for (size_t i = 0; i < nbVertex; i++)
	{
		EMDVertex &v = vertices.at(i);

		fbxControlPointsPosition[i] = FbxVector4(v.pos_x, v.pos_y, v.pos_z);

		if (fbxGeometryElementNormal)
			fbxGeometryElementNormal->GetDirectArray().Add( FbxVector4(v.norm_x, v.norm_y, v.norm_z) );
		if (fbxGeometryElementTangent)
			fbxGeometryElementTangent->GetDirectArray().Add(FbxVector4(v.tang_x, v.tang_y, v.tang_z));

		if (fbxGeometryElementUV)
			fbxGeometryElementUV->GetDirectArray().Add(FbxVector2(v.text_u, 1.0 - v.text_v));
		if (fbxGeometryElementUV2)
			fbxGeometryElementUV2->GetDirectArray().Add(FbxVector2(v.text2_u, 1.0 - v.text2_v));

		if (fbxGeometryElementColor)
		{
			v.getColorRGBAFloat(r, g, b, a);
			fbxGeometryElementColor->GetDirectArray().Add(FbxColor((double)r, (double)g, (double)b, (double)a));
		}
	}



	//Export Faces/triangle/IndexBuffer.
	size_t nbFaces = 0;
	vector<FbxCluster*> triangle_shortBoneListCluster;					//triangle use a short list of bone (not all bone per material), so we make a list of index from the real list of bones
	
	vector<FbxCluster*> clusterList;
	vector<string> boneAllareadyHaveClusterList;
	size_t nbBones;
	size_t nbAllBones = global_fbx_bones.size();
	

	size_t nbTriangles = triangles.size();
	for (size_t i = 0; i < nbTriangles; i++)
	{
		std::vector<unsigned int> &faces = triangles.at(i).faces;


		// Skinning prepartion part : make a list of bones for triangle.
		triangle_shortBoneListCluster.clear();
		if (flags & EMD_VTX_FLAG_BLEND_WEIGHT)
		{
			std::vector<std::string> &bone_names = triangles.at(i).bone_names;

			if (forceBoneNode)
			{
				bone_names.clear();
				bone_names.push_back(forceBoneNode->GetName());

				/*
				//Test because problem on declaration of parent on bindpose , for 3dsmax.
				FbxNode* bone_tmp = forceBoneNode->GetParent();
				while (bone_tmp)
				{
					string name_tmp = string(bone_tmp->GetName());
					if(name_tmp.length())
						bone_names.push_back(name_tmp);

					bone_tmp = bone_tmp->GetParent();
				}
				*/
			}


			FbxCluster* fbxCluster;
			nbBones = bone_names.size();
			for (size_t j = 0; j < nbBones; j++)
			{
				string &bone_name = bone_names.at(j);

				//avoid Cluster double
				size_t nbCluster = boneAllareadyHaveClusterList.size();
				bool isfound = false;
				for (size_t k = 0; k < nbCluster; k++)
				{
					if (boneAllareadyHaveClusterList.at(k) == bone_name)
					{
						triangle_shortBoneListCluster.push_back(clusterList.at(k));
						isfound = true;
						break;
					}
				}
				if (isfound)
					continue;


				//Create the Fbx cluster for the skin (same if there isn't bones, you could have influence)
				fbxCluster = FbxCluster::Create(scene, bone_name.c_str());
				fbxCluster->SetLinkMode(FbxCluster::eTotalOne);
				fbxCluster->SetTransformMatrix(scene->GetRootNode()->EvaluateGlobalTransform());
				lSkin->AddCluster(fbxCluster);						//attache to Skin
				
				clusterList.push_back(fbxCluster);
				boneAllareadyHaveClusterList.push_back(bone_name);
				triangle_shortBoneListCluster.push_back(fbxCluster);
				

				if (global_fbx_bones.size() != 0)
				{
					for (size_t k = 0; k < nbAllBones; k++)
					{
						if (bone_name == global_fbx_bones.at(k).mName)
						{
							ESK::FbxBonesInstance_DBxv	&fbxBoneInst = global_fbx_bones.at(k);
							fbxCluster->SetLink(fbxBoneInst.mNode);
							
							
							if (!forceBoneNode)
							{
								fbxCluster->SetTransformLinkMatrix(fbxBoneInst.mNode->EvaluateGlobalTransform());
							}else {
								FbxAMatrix matrix;
								matrix.SetIdentity();
								fbxCluster->SetTransformLinkMatrix(matrix);
							}

							break;
						}
					}
				}
			}
		}



		

		size_t bone_index;
		nbFaces = faces.size();
		for (size_t j = 0; j+2 < nbFaces; j += 3)
		{
			if ((faces.at(j) >= nbVertex) || (faces.at(j+1) >= nbVertex) || (faces.at(j+2) >= nbVertex))
			{
				printf("Problem on submesh %s : triangle/face/polygone/IndexBuffer target a wrong index %i (or %i or %i) into the vertexlist (nbvertex: %i). may be corrupt file. just skip polygone.\n", name.c_str(), faces.at(j), faces.at(j+1), faces.at(j+2), nbVertex);
				notifyError();
				continue;
			}


			//make polygone
			fbxMesh->BeginPolygon(0);

			fbxMesh->AddPolygon(faces.at(j));
			fbxMesh->AddPolygon(faces.at(j + 1));
			fbxMesh->AddPolygon(faces.at(j + 2));

			fbxMesh->EndPolygon();



			// Skinning Apply on Vertex
			if (!(flags & EMD_VTX_FLAG_BLEND_WEIGHT))
				continue;

			
			for (size_t k = 0; k < 3; k++)
			{
				EMDVertex &v = vertices.at(faces.at(j + k));

				

				/*
				//version witch work well with Blender, but there is weird things on Unity. that strange. fbx -> blender -> fbx solve this for Unity (but you loose material).
				for (size_t m = 0; m < 4; m++)								//check the 4 bone influences.
				//for (int m = 3; m >= 0; m--)				//test from check difference between fbx from emdfbx and the cleaned version from blender reexport.
				{
					//if (v.blend_weight[m] <= 0.0f)
					//	continue;
					
					bone_index = v.blend[m];
					if(bone_index >= triangle_shortBoneListCluster.size())
					{
						printf("Invalid Bone Index %d compared to Bone FBX Index Map size %d. just skip.\n", bone_index, triangle_shortBoneListCluster.size());
						notifyError();
						continue;
					}

					float blend_weight = ((m != 3) ? v.blend_weight[m] : (1.0f - (v.blend_weight[0] + v.blend_weight[1] + v.blend_weight[2])));

					triangle_shortBoneListCluster.at(bone_index)->AddControlPointIndex(faces.at(j + k), blend_weight);
				}
				*/
				
				
				
				//test version better for 3dsmax, but still trouble on unity
				std::vector<size_t> blendIndiceUnique;
				std::vector<float> blendweightlinked;
				for (size_t m = 0; m < 4; m++)				//check the 4 bones influences.
				{
					bone_index = v.blend[m];
					if (bone_index >= triangle_shortBoneListCluster.size())
					{
						printf("Invalid Bone Index %d compared to Bone FBX Index Map size %d. just skip.\n", bone_index, triangle_shortBoneListCluster.size());
						notifyError();
						continue;
					}
					float blend_weight = ((m != 3) ? v.blend_weight[m] : (1.0f - (v.blend_weight[0] + v.blend_weight[1] + v.blend_weight[2])));					

					if (forceBoneNode)
					{
						bone_index = 0;
						blend_weight = (m==0) ? 1.0f : 0.0f;
					}


					bool isfound_tmp = false;
					size_t nbIndex_tmp = blendIndiceUnique.size();
					for (size_t n = 0; n < nbIndex_tmp; n++)
					{
						if (blendIndiceUnique.at(n) == bone_index)
						{
							isfound_tmp = true;
							blendweightlinked.at(n) += blend_weight;
							break;
						}
					}
					if (!isfound_tmp)
					{
						blendIndiceUnique.push_back(bone_index);
						blendweightlinked.push_back(blend_weight);
					}
				}

				size_t nbIndex_tmp = blendIndiceUnique.size();
				for (size_t n = 0; n < nbIndex_tmp; n++)
					triangle_shortBoneListCluster.at(blendIndiceUnique.at(n))->AddControlPointIndex(faces.at(j + k), blendweightlinked.at(n));
				////////////
			}

		}
	}
}

/*-------------------------------------------------------------------------------\
|                             CreateTableEntry									 |
\-------------------------------------------------------------------------------*/
void CreateTableEntry(FbxBindingTable* pTable, FbxProperty& pProp)
{
	// create entry
	FbxBindingTableEntry& lEntry = pTable->AddNewEntry();
	// set src to the fbx property, in this sample, fbx properties have the same name with shader parameters
	FbxPropertyEntryView lSrc(&lEntry, true, true);
	// Because CgFX uses compound property, so do not use pProp.GetName()
	lSrc.SetProperty(pProp.GetHierarchicalName());

	// set dst to the shader parameter
	FbxSemanticEntryView lDst(&lEntry, false, true);
	lDst.SetSemantic(pProp.GetName());
}
/*-------------------------------------------------------------------------------\
|                             CustomParameterProp								 |
\-------------------------------------------------------------------------------*/
void CustomParameterProp(FbxSurfaceMaterial* lMaterial, FbxBindingTable* lTable, string paramName, FbxDouble4 val)
{
	FbxProperty lProp = FbxProperty::Create(lMaterial, FbxDouble4DT, paramName.c_str(), paramName.c_str());
	lProp.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
	lProp.Set(val);
	CreateTableEntry(lTable, lProp);
}
/*-------------------------------------------------------------------------------\
|                             CustomTextureProp									 |
\-------------------------------------------------------------------------------*/
void CustomTextureProp(FbxScene *scene, FbxSurfaceMaterial* lMaterial, FbxBindingTable* lTable, string paramName, string filename)
{
	FbxProperty lProp = FbxProperty::Create(lMaterial, FbxDouble3DT, paramName.c_str(), paramName.c_str());
	lProp.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
	FbxDouble3 lMapVal(0, 1, 0);
	lProp.Set(lMapVal);
	CreateTableEntry(lTable, lProp);

	FbxFileTexture* lTexture = FbxFileTexture::Create(scene, paramName.c_str());
	if (filename.size() != 0)
	{
		if (LibXenoverse::fileCheck(filename))
			lTexture->SetFileName(filename.c_str());
		else
			printf("Image %s not found\n", filename.c_str());
	}
	lTexture->SetTextureUse(FbxTexture::eStandard);
	lTexture->SetMappingType(FbxTexture::eUV);
	lTexture->ConnectDstProperty(lProp);
}
/*-------------------------------------------------------------------------------\
|                             exportFBXMaterial									 |
\-------------------------------------------------------------------------------*/
FbxSurfaceMaterial* EMDSubmesh::exportFBXMaterial(FbxScene *scene, string material_name, std::vector<EMB*> listTexturePackEMB, EMM* emmMaterial, bool wantNoTexture)
{
	
	
	if (wantNoTexture)						//case with a basic material, with no texture.
	{
		printf("Some problem of loading into blender a fbx using fbxMaterial with shader, may oblige to use classical material with only diffuse map. for material %s\n", material_name.c_str());

		FbxSurfacePhong* fbxMaterial = FbxSurfacePhong::Create(scene, FbxString(material_name.c_str()).Buffer());

		fbxMaterial->ShadingModel.Set(FbxString("Phong"));
		fbxMaterial->Emissive.Set(FbxDouble3(0.0, 0.0, 0.0));
		fbxMaterial->Ambient.Set(FbxDouble3(0.0, 0.0, 0.0));
		fbxMaterial->AmbientFactor.Set(1.0);
		fbxMaterial->Diffuse.Set(FbxDouble3(1.0, 1.0, 1.0));
		fbxMaterial->DiffuseFactor.Set(1.0);
		fbxMaterial->Shininess.Set(0.0);
		fbxMaterial->Specular.Set(FbxDouble3(0.0, 0.0, 0.0));
		fbxMaterial->SpecularFactor.Set(0.0);

		return (FbxSurfaceMaterial*)fbxMaterial;
	}
	
	
	
	
	
	//search current material.
	EMMMaterial* emm_mat = 0;
	float dytLineIndex = 0.0f;
	if (emmMaterial != NULL)
	{
		size_t isFound = (size_t)-1;

		vector<EMMMaterial *> listMaterial = emmMaterial->getMaterials();
		for (size_t i = 0; i < listMaterial.size(); i++)							//search for material definition
		{
			if (listMaterial.at(i)->getName() == material_name)
			{
				isFound = i;
				break;
			}
		}

		if (isFound != (size_t)-1)
		{
			emm_mat = emmMaterial->getMaterials().at(isFound);
			EMMParameter* parameter = emm_mat->getParameter("MatScale1X");

			if(parameter)
				dytLineIndex = parameter->float_value;
		}
	}





	//a problem on using shader into blender via fbx definition, may crash the loading of fbx file into blender. 
	// so we only use a classic Material of Fbx, with some texture.
	//if (!emmMaterial)										
	{
		FbxSurfacePhong* fbxMaterial = FbxSurfacePhong::Create(scene, FbxString(material_name.c_str()).Buffer());

		fbxMaterial->ShadingModel.Set(FbxString("Phong"));
		fbxMaterial->Emissive.Set( FbxDouble3(0.0, 0.0, 0.0) );
		fbxMaterial->Ambient.Set( FbxDouble3(0.0, 0.0, 0.0) );
		fbxMaterial->AmbientFactor.Set(1.0);
		fbxMaterial->Diffuse.Set( FbxDouble3(1.0, 1.0, 1.0) );
		fbxMaterial->DiffuseFactor.Set(1.0);
		fbxMaterial->Shininess.Set(0.0);
		fbxMaterial->Specular.Set( FbxDouble3(0.0, 0.0, 0.0) );
		fbxMaterial->SpecularFactor.Set(0.0);
		
		size_t nbDefs = definitions.size();
		bool diffuseAssigned = false;
		fbxMaterial->TransparencyFactor.Set(0.0);
		for(size_t i=0;i<nbDefs;i++)
		{
			EMDTextureUnitState &def = definitions.at(i);
			string indexStr = std::to_string(def.texIndex);

			// Build source filename (DATA###.dds)
			char dataName[20];
			sprintf(dataName, "DATA%03u.dds", def.texIndex);

			std::string sourceFile = listTexturePackEMB.size() >= 2
				? listTexturePackEMB.at(1)->getName() + "\\" + dataName
				: "";

			// Destination filename
			std::string destFile = listTexturePackEMB.size() >= 2
				? listTexturePackEMB.at(1)->getName() + "\\" + material_name + "_" + indexStr + ".dds"
				: "";

			// Copy texture if it exists
			if (!sourceFile.empty())
			{
				std::ifstream srce(sourceFile, std::ios::binary);
				if (srce)
				{
					std::ofstream dest(destFile, std::ios::binary);
					dest << srce.rdbuf();
					srce.close();
					dest.close();

					// Create FBX texture node
					FbxFileTexture* texture = FbxFileTexture::Create(fbxMaterial, (material_name + "_" + indexStr).c_str());
					texture->SetFileName(destFile.c_str());
					texture->SetTextureUse(FbxTexture::eStandard);
					texture->SetMappingType(FbxTexture::eUV);
					texture->SetMaterialUse(FbxFileTexture::eModelMaterial);
					texture->SetSwapUV(false);
					texture->SetTranslation(0.0, 0.0);
					texture->SetScale(def.textScale_u, def.textScale_v);
					texture->SetRotation(0.0, 0.0);

					if (!diffuseAssigned)
					{
						// Assign to diffuse/base color
						fbxMaterial->Diffuse.ConnectSrcObject(texture);
						diffuseAssigned = true;
					}
					else
					{
						// Just add as an extra property so it’s included in the FBX
						//std::string extraPropName = "ExtraTexture_" + indexStr;
						//FbxProperty prop_extra = FbxProperty::Create(fbxMaterial, FbxStringDT, extraPropName.c_str());
						//prop_extra.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
						//prop_extra.Set<FbxString>(destFile.c_str());
					}
				}
			}

			printf("  Material %s:\n", material_name.c_str());
			printf("  Definition %u:\n", (unsigned int)i);
			printf("    flag0: %u\n", def.flag0);
			printf("    texIndex: %u\n", def.texIndex);
			printf("    adressMode_u: %u\n", def.adressMode_u);
			printf("    adressMode_v: %u\n", def.adressMode_v);
			printf("    filtering_minification: %f\n", def.filtering_minification);
			printf("    filtering_magnification: %f\n", def.filtering_magnification);
			printf("    textScale_u: %f\n", def.textScale_u);
			printf("    textScale_v: %f\n", def.textScale_v);

			std::string baseName = "SubmeshDef_" + material_name + "_" + std::to_string(i) + "_";

			FbxProperty prop_flag0 = FbxProperty::Create(fbxMaterial, FbxIntDT, (baseName + "flag0").c_str());
			prop_flag0.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
			prop_flag0.Set<int>(def.flag0);

			FbxProperty prop_texIndex = FbxProperty::Create(fbxMaterial, FbxIntDT, (baseName + "texIndex").c_str());
			prop_texIndex.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
			prop_texIndex.Set<int>(def.texIndex);

			FbxProperty prop_adressMode_u = FbxProperty::Create(fbxMaterial, FbxIntDT, (baseName + "adressMode_u").c_str());
			prop_adressMode_u.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
			prop_adressMode_u.Set<int>(def.adressMode_u);

			FbxProperty prop_adressMode_v = FbxProperty::Create(fbxMaterial, FbxIntDT, (baseName + "adressMode_v").c_str());
			prop_adressMode_v.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
			prop_adressMode_v.Set<int>(def.adressMode_v);

			FbxProperty prop_filtering_minification = FbxProperty::Create(fbxMaterial, FbxDoubleDT, (baseName + "filtering_minification").c_str());
			prop_filtering_minification.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
			prop_filtering_minification.Set<double>(def.filtering_minification);

			FbxProperty prop_filtering_magnification = FbxProperty::Create(fbxMaterial, FbxDoubleDT, (baseName + "filtering_magnification").c_str());
			prop_filtering_magnification.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
			prop_filtering_magnification.Set<double>(def.filtering_magnification);

			FbxProperty prop_textScale_u = FbxProperty::Create(fbxMaterial, FbxDoubleDT, (baseName + "texScale_u").c_str());
			prop_textScale_u.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
			prop_textScale_u.Set<double>(def.textScale_u);

			FbxProperty prop_textScale_v = FbxProperty::Create(fbxMaterial, FbxDoubleDT, (baseName + "texScale_v").c_str());
			prop_textScale_v.ModifyFlag(FbxPropertyFlags::eUserDefined, true);
			prop_textScale_v.Set<double>(def.textScale_v);
		}
		

		return (FbxSurfaceMaterial*)fbxMaterial;
	}
}




#endif
}