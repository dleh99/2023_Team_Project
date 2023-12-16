using NUnit.Framework.Internal;
using System.Collections;
using System.Collections.Generic;
using System.IO;
//using System.Numerics;
using System.Runtime.Serialization.Json;
using Unity.VisualScripting;
using UnityEditor.Timeline;
using UnityEngine;

public class Exporter : MonoBehaviour
{
    public AnimationClip[] m_AnimationClips;

    private SkinnedMeshRenderer[] m_pSkinnedMeshRenderers = null;
    
    private int m_nFrames = 0;

    StreamWriter textWriter = null;
    BinaryWriter binaryWriter = null;
    BinaryWriter animationBinaryWriter = null;

    void WriteMaterialsText(Material[] materials)
    {
        textWriter.Write('\n');
        textWriter.Write("<Materials>:");
        textWriter.Write(materials.Length);
        textWriter.Write('\n');

        for (int i = 0; i < materials.Length; ++i)
        {
            textWriter.Write(i);

            // Color
            if (materials[i].HasProperty("_Color"))
            {
                Color albedo = materials[i].GetColor("_Color");
                textWriter.Write(" albedo ");
                textWriter.Write(albedo.r);
                textWriter.Write(albedo.g);
                textWriter.Write(albedo.b);
                textWriter.Write(albedo.a);
                textWriter.Write(' ');
            }
            if (materials[i].HasProperty("_EmissionColor"))
            {
                Color emission = materials[i].GetColor("_EmissionColor");
                textWriter.Write("emission ");
                textWriter.Write(emission.r);
                textWriter.Write(emission.g);
                textWriter.Write(emission.b);
                textWriter.Write(emission.a);
                textWriter.Write(' ');
            }
            if (materials[i].HasProperty("_SpecColor"))
            {
                Color specular = materials[i].GetColor("_SpecColor");
                textWriter.Write("specular ");
                textWriter.Write(specular.r);
                textWriter.Write(specular.g);
                textWriter.Write(specular.b);
                textWriter.Write(specular.a);
                textWriter.Write(' ');
            }

            // Float
            if (materials[i].HasProperty("_Glossiness"))
            {
                textWriter.Write("_Glossiness");
                textWriter.Write('\n');
            }
            if (materials[i].HasProperty("_Smoothness"))
            {
                textWriter.Write("_Smoothness");
                textWriter.Write('\n');
            }
            if (materials[i].HasProperty("_Metalic"))
            {
                textWriter.Write("_Metalic");
                textWriter.Write('\n');
            }
            if (materials[i].HasProperty("_SpecularHighlights"))
            {
                textWriter.Write("_SpecularHighlights");
                textWriter.Write('\n');
            }
            if (materials[i].HasProperty("_GlossyReflections"))
            {
                textWriter.Write("_GlossyReflections");
                textWriter.Write('\n');
            }

            // Texture
            if (materials[i].HasProperty("_MainTex"))
            {
                textWriter.Write("_MainTex ");

                Texture mainAlbedoMap = materials[i].GetTexture("_MainTex");
                if (mainAlbedoMap)
                {
                    textWriter.Write(string.Copy(mainAlbedoMap.name).Replace(" ", "_"));
                }
                else
                {
                    textWriter.Write("MainAlbedoMapNull ");
                }
                textWriter.Write('\n');
            }
            if (materials[i].HasProperty("_SpecGlossMap "))
            {
                textWriter.Write("_SpecGlossMap");
                textWriter.Write('\n');
            }
            if (materials[i].HasProperty("_MetallicGlossMap"))
            {
                textWriter.Write("_MetallicGlossMap");
                textWriter.Write('\n');
            }
            if (materials[i].HasProperty("_BumpMap"))
            {
                textWriter.Write("_BumpMap");
                textWriter.Write('\n');
            }
            if (materials[i].HasProperty("_EmissionMap"))
            {
                textWriter.Write("_EmissionMap");
                textWriter.Write('\n');
            }
            if (materials[i].HasProperty("_DetailAlbedoMap"))
            {
                textWriter.Write("_DetailAlbedoMap");
                textWriter.Write('\n');
            }
            if (materials[i].HasProperty("_DetailNormalMap"))
            {
                textWriter.Write("_DetailNormalMap");
                textWriter.Write('\n');
            }
        }

        textWriter.Write('m');
    }

    void WriteFrameText(Transform obj)
    {
        textWriter.Write("<Frame>: ");
        textWriter.Write(m_nFrames++);
        textWriter.Write(' ');
        textWriter.Write(obj.name);
        textWriter.Write('\n');

        // Transform
        textWriter.Write("Transform: (");
        textWriter.Write(obj.localPosition.x);
        textWriter.Write(", ");
        textWriter.Write(obj.localPosition.y);
        textWriter.Write(", ");
        textWriter.Write(obj.localPosition.z);
        textWriter.Write(')');

        textWriter.Write(' ');

        textWriter.Write("Euler Angles: (");
        textWriter.Write(obj.localEulerAngles.x);
        textWriter.Write(", ");
        textWriter.Write(obj.localEulerAngles.y);
        textWriter.Write(", ");
        textWriter.Write(obj.localEulerAngles.z);
        textWriter.Write(')');

        textWriter.Write(' ');

        textWriter.Write("Scale: (");
        textWriter.Write(obj.localScale.x);
        textWriter.Write(", ");
        textWriter.Write(obj.localScale.y);
        textWriter.Write(", ");
        textWriter.Write(obj.localScale.z);
        textWriter.Write(')');

        textWriter.Write(' ');

        textWriter.Write("Rotation: (");
        textWriter.Write(obj.localRotation.x);
        textWriter.Write(", ");
        textWriter.Write(obj.localRotation.y);
        textWriter.Write(", ");
        textWriter.Write(obj.localRotation.z);
        textWriter.Write(", ");
        textWriter.Write(obj.localRotation.w);
        textWriter.Write(')');

        textWriter.Write('\n');

        // 4x4Matrix
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(obj.localPosition, obj.localRotation, obj.localScale);

        textWriter.Write("4x4 Matrix: ");
        textWriter.Write('\n');
        textWriter.Write(matrix.m00);
        textWriter.Write(' ');
        textWriter.Write(matrix.m10);
        textWriter.Write(' ');
        textWriter.Write(matrix.m20);
        textWriter.Write(' ');
        textWriter.Write(matrix.m30);
        textWriter.Write('\n');
        textWriter.Write(matrix.m01);
        textWriter.Write(' ');
        textWriter.Write(matrix.m11);
        textWriter.Write(' ');
        textWriter.Write(matrix.m21);
        textWriter.Write(' ');
        textWriter.Write(matrix.m31);
        textWriter.Write('\n');
        textWriter.Write(matrix.m02);
        textWriter.Write(' ');
        textWriter.Write(matrix.m12);
        textWriter.Write(' ');
        textWriter.Write(matrix.m22);
        textWriter.Write(' ');
        textWriter.Write(matrix.m32);
        textWriter.Write('\n');
        textWriter.Write(matrix.m03);
        textWriter.Write(' ');
        textWriter.Write(matrix.m13);
        textWriter.Write(' ');
        textWriter.Write(matrix.m23);
        textWriter.Write(' ');
        textWriter.Write(matrix.m33);
        textWriter.Write('\n');

        MeshFilter modelMeshFilter = obj.GetComponent<MeshFilter>();
        MeshRenderer modelMeshRenderer = obj.GetComponent<MeshRenderer>();

        if (modelMeshFilter && modelMeshRenderer)
        {
            textWriter.Write("<Mesh>: ");
            WriteMeshText(modelMeshFilter);

            Material[] materials = modelMeshRenderer.materials;
            if (materials.Length > 0)
            {
                WriteMaterialsText(materials);
            }
        }
        else
        {
            SkinnedMeshRenderer modelSkinnenMeshRenderer = obj.GetComponent<SkinnedMeshRenderer>();
            if (modelSkinnenMeshRenderer)
            {
                textWriter.Write("<Mesh>: ");
                WriteSkinnedMeshText(modelSkinnenMeshRenderer);

                Material[] materials = modelSkinnenMeshRenderer.materials;
                if (materials.Length > 0)
                {
                    WriteMaterialsText(materials);
                }
            }
        }
    }

    void WriteFrameHierarachyText(Transform obj)
    {
        WriteFrameText(obj);

        textWriter.Write("<Children>:");
        textWriter.Write(obj.childCount);

        textWriter.Write('\n');

        for (int i = 0; i < obj.childCount; i++)
        {
            WriteFrameHierarachyText(obj.GetChild(i));
        }

        textWriter.Write('H');

        Debug.Log("Model Text Write Done!!!");
    }

    void WriteMeshText(MeshFilter meshFilter)
    {
        if (meshFilter != null)
        {
            int verticesLength = meshFilter.mesh.vertices.Length;
            int normalsLength = meshFilter.mesh.normals.Length;
            int tangentsLength = meshFilter.mesh.tangents.Length;
            int uv0Length = meshFilter.mesh.uv.Length;
            uint indexLength = meshFilter.mesh.GetIndexCount(0);

            textWriter.Write('\n');
            textWriter.Write("<Vertex>: ");
            textWriter.Write(verticesLength);
            textWriter.Write(' ');
            for (int j = 0; j < verticesLength; ++j)
            {
                textWriter.Write(meshFilter.mesh.vertices[j].x);
                textWriter.Write(meshFilter.mesh.vertices[j].y);
                textWriter.Write(meshFilter.mesh.vertices[j].z);
                textWriter.Write(' ');
            }

            textWriter.Write('\n');
            textWriter.Write("<Normal>: ");
            textWriter.Write(normalsLength);
            textWriter.Write(' ');
            for (int j = 0; j < normalsLength; ++j)
            {
                textWriter.Write(meshFilter.mesh.normals[j].x);
                textWriter.Write(meshFilter.mesh.normals[j].y);
                textWriter.Write(meshFilter.mesh.normals[j].z);
                textWriter.Write(' ');
            }

            textWriter.Write('\n');
            textWriter.Write("<Tangent>: ");
            textWriter.Write(tangentsLength);
            textWriter.Write(' ');
            for (int j = 0; j < tangentsLength; ++j)
            {
                textWriter.Write(meshFilter.mesh.tangents[j].x);
                textWriter.Write(meshFilter.mesh.tangents[j].y);
                textWriter.Write(meshFilter.mesh.tangents[j].z);
                textWriter.Write(meshFilter.mesh.tangents[j].w);
                textWriter.Write(' ');
            }

            textWriter.Write('\n');
            textWriter.Write("<UV>: ");
            textWriter.Write(uv0Length);
            textWriter.Write(' ');
            for (int j = 0; j < uv0Length; ++j)
            {
                textWriter.Write(meshFilter.mesh.uv[j].x);
                textWriter.Write(meshFilter.mesh.uv[j].y);
                textWriter.Write(' ');
            }

            textWriter.Write('\n');
            textWriter.Write("<Index>: ");
            textWriter.Write(indexLength);
            textWriter.Write(' ');
            for (int j = 0; j < indexLength; ++j)
            {
                textWriter.Write(meshFilter.mesh.GetIndices(0)[j]);
                textWriter.Write(' ');
            }

            textWriter.Write("\n");
            //textWriter.Write("<Bone>: ");
            //textWriter.Write(meshFilter.mesh.boneWeights.Length);
            //textWriter.Write(" ");
            //for(int j=0;j< meshFilter.mesh.boneWeights.Length; ++j)
            //{
            //    textWriter.Write('(');
            //    textWriter.Write(meshFilter.mesh.boneWeights[j].boneIndex0);
            //    textWriter.Write(", ");
            //    textWriter.Write(meshFilter.mesh.boneWeights[j].weight0);
            //    textWriter.Write(')');
            //    textWriter.Write(" ");
            //}
        }
    }

    void WriteSkinnedMeshText(SkinnedMeshRenderer skinnedMeshRenderer)
    {
        Mesh skinnedMesh = skinnedMeshRenderer.sharedMesh;

        int verticesLength = skinnedMesh.vertices.Length;
        int normalsLength = skinnedMesh.normals.Length;
        int tangentsLength = skinnedMesh.tangents.Length;
        int uv0Length = skinnedMesh.uv.Length;
        uint indexLength = skinnedMesh.GetIndexCount(0);

        textWriter.Write('\n');
        textWriter.Write("<Vertex>: ");
        textWriter.Write(' ');
        textWriter.Write(verticesLength);
        for (int j = 0; j < verticesLength; ++j)
        {
            textWriter.Write(skinnedMesh.vertices[j].x);
            textWriter.Write(skinnedMesh.vertices[j].y);
            textWriter.Write(skinnedMesh.vertices[j].z);
            textWriter.Write(' ');
        }

        textWriter.Write('\n');
        textWriter.Write("<Normal>: ");
        textWriter.Write(' ');
        textWriter.Write(normalsLength);
        for (int j = 0; j < normalsLength; ++j)
        {
            textWriter.Write(skinnedMesh.normals[j].x);
            textWriter.Write(skinnedMesh.normals[j].y);
            textWriter.Write(skinnedMesh.normals[j].z);
            textWriter.Write(' ');
        }

        textWriter.Write('\n');
        textWriter.Write("<Tangent>: ");
        textWriter.Write(' ');
        textWriter.Write(tangentsLength);
        for (int j = 0; j < tangentsLength; ++j)
        {
            textWriter.Write(skinnedMesh.tangents[j].x);
            textWriter.Write(skinnedMesh.tangents[j].y);
            textWriter.Write(skinnedMesh.tangents[j].z);
            textWriter.Write(skinnedMesh.tangents[j].w);
            textWriter.Write(' ');
        }

        textWriter.Write('\n');
        textWriter.Write("<UV>: ");
        textWriter.Write(' ');
        textWriter.Write(uv0Length);
        for (int j = 0; j < uv0Length; ++j)
        {
            textWriter.Write(skinnedMesh.uv[j].x);
            textWriter.Write(skinnedMesh.uv[j].y);
            textWriter.Write(' ');
        }

        textWriter.Write('\n');
        textWriter.Write("<Index>: ");
        textWriter.Write(' ');
        textWriter.Write(indexLength);
        for (int j = 0; j < indexLength; ++j)
        {
            textWriter.Write(skinnedMesh.GetIndices(0)[j]);
            textWriter.Write(' ');
        }
    }

    void WriteAnimationMatrixHierarchyText(Transform current)
    {
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(current.localPosition, current.localRotation, current.localScale);

        textWriter.Write('\n');
        textWriter.Write(matrix.m00);
        textWriter.Write(' ');
        textWriter.Write(matrix.m10);
        textWriter.Write(' ');
        textWriter.Write(matrix.m20);
        textWriter.Write(' ');
        textWriter.Write(matrix.m30);
        textWriter.Write('\n');
        textWriter.Write(matrix.m01);
        textWriter.Write(' ');
        textWriter.Write(matrix.m11);
        textWriter.Write(' ');
        textWriter.Write(matrix.m21);
        textWriter.Write(' ');
        textWriter.Write(matrix.m31);
        textWriter.Write('\n');
        textWriter.Write(matrix.m02);
        textWriter.Write(' ');
        textWriter.Write(matrix.m12);
        textWriter.Write(' ');
        textWriter.Write(matrix.m22);
        textWriter.Write(' ');
        textWriter.Write(matrix.m32);
        textWriter.Write('\n');
        textWriter.Write(matrix.m03);
        textWriter.Write(' ');
        textWriter.Write(matrix.m13);
        textWriter.Write(' ');
        textWriter.Write(matrix.m23);
        textWriter.Write(' ');
        textWriter.Write(matrix.m33);
        textWriter.Write('\n');

        if (current.childCount > 0)
        {
            for (int i = 0; i < current.childCount; ++i)
            {
                WriteAnimationMatrixHierarchyText(current.GetChild(i));
            }
        }
    }

    void WriteAnimationTransformsText(string str, int nKeyFrame, float fKeyFrameTime)
    {
        textWriter.Write(str);
        textWriter.Write(" Key Frame: ");
        textWriter.Write(nKeyFrame);
        textWriter.Write(" Key Frame Time: ");
        textWriter.Write(fKeyFrameTime);
        textWriter.Write('\n');

        WriteAnimationMatrixHierarchyText(transform);
    }

    void WriteFrameNameHierarchyText(Transform obj)
    {
        textWriter.Write((obj.gameObject) ? string.Copy(obj.gameObject.name).Replace(" ", "_") : "null");
        textWriter.Write('\n');

        if (obj.childCount > 0)
        {
            for (int i = 0; i < obj.childCount; ++i)
                WriteFrameNameHierarchyText(obj.GetChild(i));
        }
    }

    void WriteFrameNamesText(string str)
    {
        textWriter.Write(str);
        textWriter.Write(' ');
        textWriter.Write(m_nFrames);
        textWriter.Write(' ');

        WriteFrameNameHierarchyText(transform);
    }

    void WriteAnimationClipsInfoText()
    {
        textWriter.Write("<AnimationSets>: ");
        textWriter.Write(m_AnimationClips.Length);
        textWriter.Write('\n');

        WriteFrameNamesText("<FrameNames>: ");

        for (int i = 0; i < m_AnimationClips.Length; ++i)
        {
            int nFramesPerSec = (int)m_AnimationClips[i].frameRate;
            int nKeyFrames = Mathf.CeilToInt(m_AnimationClips[i].length * nFramesPerSec);

            textWriter.Write('\n');
            textWriter.Write("<AnimationSet>: ");
            textWriter.Write(i);
            textWriter.Write(' ');
            textWriter.Write((m_AnimationClips[i]) ? string.Copy(m_AnimationClips[i].name).Replace(" ", "_") : "null");
            textWriter.Write('\n');
            textWriter.Write(" Animation Clip Length: ");
            textWriter.Write(m_AnimationClips[i].length);
            textWriter.Write('\n');
            textWriter.Write(" Animation Clip Frame Per Sec: ");
            textWriter.Write(nFramesPerSec);
            textWriter.Write('\n');
            textWriter.Write(" Animation Clip KeyFrames : ");
            textWriter.Write(nKeyFrames);
            textWriter.Write('\n');

            float fFrameRate = (1.0f / nFramesPerSec);
            float fKeyFrameTime = 0.0f;

            for (int j = 0; j < nKeyFrames; ++j)
            {
                m_AnimationClips[i].SampleAnimation(gameObject, fKeyFrameTime);

                WriteAnimationTransformsText("<Transforms>: ", j, fKeyFrameTime);

                fKeyFrameTime += fFrameRate;
            }
        }

        textWriter.Write("</AnimationSets> ");
    }

    void WriteMaterials(Material[] materials)
    {
        binaryWriter.Write('m');
        binaryWriter.Write(materials.Length);

        for(int i=0;i<materials.Length;++i)
        {
            binaryWriter.Write(i);

            // Color
            if (materials[i].HasProperty("_Color"))
            {
                Color albedo = materials[i].GetColor("_Color");
                binaryWriter.Write("albedo ");
                binaryWriter.Write(albedo.r);
                binaryWriter.Write(albedo.g);
                binaryWriter.Write(albedo.b);
                binaryWriter.Write(albedo.a);
            }
            if (materials[i].HasProperty("_EmissionColor"))
            {
                Color emission = materials[i].GetColor("_EmissionColor");
                binaryWriter.Write("emissive ");
                binaryWriter.Write(emission.r);
                binaryWriter.Write(emission.g);
                binaryWriter.Write(emission.b);
                binaryWriter.Write(emission.a);
            }
            if (materials[i].HasProperty("_SpecColor"))
            {
                Color specular = materials[i].GetColor("_SpecColor");
                binaryWriter.Write("specular ");
                binaryWriter.Write(specular.r);
                binaryWriter.Write(specular.g);
                binaryWriter.Write(specular.b);
                binaryWriter.Write(specular.a);
            }

            // Float
            if (materials[i].HasProperty("_Glossiness"))
            {
                float glossiness = materials[i].GetFloat("_Glossiness");
                binaryWriter.Write("glossiness ");
                binaryWriter.Write(glossiness);
            }
            if (materials[i].HasProperty("_Smoothness"))
            {
                float smoothness = materials[i].GetFloat("_Smoothness");
                binaryWriter.Write("smoothness ");
                binaryWriter.Write(smoothness);
            }
            if (materials[i].HasProperty("_Metallic"))
            {
                float metalic = materials[i].GetFloat("_Metallic");
                binaryWriter.Write("metalic ");
                binaryWriter.Write(metalic);
            }
            if (materials[i].HasProperty("_SpecularHighlights"))
            {
                float specularHighlights = materials[i].GetFloat("_SpecularHighlights");
                binaryWriter.Write("specularHighlights ");
                binaryWriter.Write(specularHighlights);
            }
            if (materials[i].HasProperty("_GlossyReflections"))
            {
                float glossyReflections = materials[i].GetFloat("_GlossyReflections");
                binaryWriter.Write("glossyReflections ");
                binaryWriter.Write(glossyReflections);
            }

            // Texture
            if (materials[i].HasProperty("_MainTex"))
            {
                binaryWriter.Write("<AlbedoMap> ");
                
                Texture mainAlbedoMap = materials[i].GetTexture("_MainTex");
                if(mainAlbedoMap)
                {
                    binaryWriter.Write(string.Copy(mainAlbedoMap.name).Replace(" ", "_"));
                }
                else
                {
                    binaryWriter.Write("MainAlbedoMapNull ");
                }
            }
            if (materials[i].HasProperty("_SpecGlossMap"))
            {
                binaryWriter.Write("<SpecularMap> ");

                Texture specularMap = materials[i].GetTexture("_SpecGlossMap");
                if (specularMap)
                {
                    binaryWriter.Write(string.Copy(specularMap.name).Replace(" ", "_"));
                }
                else
                {
                    binaryWriter.Write("SpecularMapNull ");
                }
            }
            if (materials[i].HasProperty("_MetallicGlossMap"))
            {
                binaryWriter.Write("<MetallicMap> ");

                Texture metallicMap = materials[i].GetTexture("_MetallicGlossMap");
                if (metallicMap)
                {
                    binaryWriter.Write(string.Copy(metallicMap.name).Replace(" ", "_"));
                }
                else
                {
                    binaryWriter.Write("MetallicMapNull ");
                }
            }
            if (materials[i].HasProperty("_BumpMap"))
            {
                binaryWriter.Write("<NormalMap> ");

                Texture bumpMap = materials[i].GetTexture("_BumpMap");
                if (bumpMap)
                {
                    binaryWriter.Write(string.Copy(bumpMap.name).Replace(" ", "_"));
                }
                else
                {
                    binaryWriter.Write("BumpMapNull ");
                }
            }
            if (materials[i].HasProperty("_EmissionMap"))
            {
                binaryWriter.Write("<EmissionMap> ");

                Texture emissionMap = materials[i].GetTexture("_EmissionMap");
                if (emissionMap)
                {
                    binaryWriter.Write(string.Copy(emissionMap.name).Replace(" ", "_"));
                }
                else
                {
                    binaryWriter.Write("EmissionMapNull ");
                }
            }
            if (materials[i].HasProperty("_DetailAlbedoMap"))
            {
                binaryWriter.Write("<DetailAlbedoMap> ");

                Texture detailAlbedoMap = materials[i].GetTexture("_DetailAlbedoMap");
                if (detailAlbedoMap)
                {
                    binaryWriter.Write(string.Copy(detailAlbedoMap.name).Replace(" ", "_"));
                }
                else
                {
                    binaryWriter.Write("DetailAlbedoMapNull ");
                }
            }
            if (materials[i].HasProperty("_DetailNormalMap"))
            {
                binaryWriter.Write("<DetailNormalMap> ");

                Texture detailNormalMap = materials[i].GetTexture("_DetailNormalMap");
                if (detailNormalMap)
                {
                    binaryWriter.Write(string.Copy(detailNormalMap.name).Replace(" ", "_"));
                }
                else
                {
                    binaryWriter.Write("DetailNormalMapNull ");
                }
            }
        }

        binaryWriter.Write("</Material> ");
    }

    void WriteFrame(Transform obj)
    {
        binaryWriter.Write('F');
        binaryWriter.Write(m_nFrames++);
        binaryWriter.Write(obj.name);

        // Transform
        binaryWriter.Write(obj.localPosition.x);
        binaryWriter.Write(obj.localPosition.y);
        binaryWriter.Write(obj.localPosition.z);

        binaryWriter.Write(obj.localEulerAngles.x);
        binaryWriter.Write(obj.localEulerAngles.y);
        binaryWriter.Write(obj.localEulerAngles.z);

        binaryWriter.Write(obj.localScale.x);
        binaryWriter.Write(obj.localScale.y);
        binaryWriter.Write(obj.localScale.z);

        binaryWriter.Write(obj.localRotation.x);
        binaryWriter.Write(obj.localRotation.y);
        binaryWriter.Write(obj.localRotation.z);
        binaryWriter.Write(obj.localRotation.w);

        // 4x4Matrix
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(obj.localPosition, obj.localRotation, obj.localScale);

        binaryWriter.Write(matrix.m00);
        binaryWriter.Write(matrix.m10);
        binaryWriter.Write(matrix.m20);
        binaryWriter.Write(matrix.m30);
        binaryWriter.Write(matrix.m01);
        binaryWriter.Write(matrix.m11);
        binaryWriter.Write(matrix.m21);
        binaryWriter.Write(matrix.m31);
        binaryWriter.Write(matrix.m02);
        binaryWriter.Write(matrix.m12);
        binaryWriter.Write(matrix.m22);
        binaryWriter.Write(matrix.m32);
        binaryWriter.Write(matrix.m03);
        binaryWriter.Write(matrix.m13);
        binaryWriter.Write(matrix.m23);
        binaryWriter.Write(matrix.m33);

        MeshFilter modelMeshFilter = obj.GetComponent<MeshFilter>();
        MeshRenderer modelMeshRenderer = obj.GetComponent<MeshRenderer>();

        if (modelMeshFilter && modelMeshRenderer)
        {
            binaryWriter.Write('M');
            WriteMesh(modelMeshFilter);

            Material[] materials = modelMeshRenderer.materials;
            if(materials.Length> 0)
            {
                WriteMaterials(materials);
            }
        }
        else
        {
            SkinnedMeshRenderer modelSkinnenMeshRenderer = obj.GetComponent<SkinnedMeshRenderer>();
            if(modelSkinnenMeshRenderer)
            {
                binaryWriter.Write('S');        // <SkinnigInfo>
                WriteSkinnedMesh(modelSkinnenMeshRenderer);

                Material[] materials = modelSkinnenMeshRenderer.materials;
                if (materials.Length > 0)
                {
                    WriteMaterials(materials);
                }
            }
        }
    }

    void WriteFrameHierarachy(Transform obj)
    {
        WriteFrame(obj);

        binaryWriter.Write('C');
        binaryWriter.Write(obj.childCount);

        for(int i=0; i<obj.childCount; i++)
        {
            WriteFrameHierarachy(obj.GetChild(i));
        }

        binaryWriter.Write('E');

        Debug.Log("Model Binary Write Done!!!");
    }

    void WriteMesh(MeshFilter meshFilter)
    {
        if (meshFilter != null)
        {
            int verticesLength = meshFilter.mesh.vertices.Length;
            int normalsLength = meshFilter.mesh.normals.Length;
            int tangentsLength = meshFilter.mesh.tangents.Length;
            int uv0Length = meshFilter.mesh.uv.Length;
            uint indexLength = meshFilter.mesh.GetIndexCount(0);

            binaryWriter.Write(verticesLength);
            for (int j = 0; j < verticesLength; ++j)
            {
                binaryWriter.Write(meshFilter.mesh.vertices[j].x);
                binaryWriter.Write(meshFilter.mesh.vertices[j].y);
                binaryWriter.Write(meshFilter.mesh.vertices[j].z);
            }

            binaryWriter.Write(normalsLength);
            for (int j = 0; j < normalsLength; ++j)
            {
                binaryWriter.Write(meshFilter.mesh.normals[j].x);
                binaryWriter.Write(meshFilter.mesh.normals[j].y);
                binaryWriter.Write(meshFilter.mesh.normals[j].z);
            }

            binaryWriter.Write(tangentsLength);
            for (int j = 0; j < tangentsLength; ++j)
            {
                binaryWriter.Write(meshFilter.mesh.tangents[j].x);
                binaryWriter.Write(meshFilter.mesh.tangents[j].y);
                binaryWriter.Write(meshFilter.mesh.tangents[j].z);
                binaryWriter.Write(meshFilter.mesh.tangents[j].w);
            }

            binaryWriter.Write(uv0Length);
            for (int j = 0; j < uv0Length; ++j)
            {
                binaryWriter.Write(meshFilter.mesh.uv[j].x);
                binaryWriter.Write(meshFilter.mesh.uv[j].y);
            }

            binaryWriter.Write(indexLength);

            for (int j = 0; j < indexLength; ++j)
            {
                binaryWriter.Write(meshFilter.mesh.GetIndices(0)[j]);
            }
        }
    }

    void WriteSkinnedMesh(SkinnedMeshRenderer skinnedMeshRenderer)
    {
        Mesh skinnedMesh = skinnedMeshRenderer.sharedMesh;

        int verticesLength = skinnedMesh.vertices.Length;
        int normalsLength = skinnedMesh.normals.Length;
        int tangentsLength = skinnedMesh.tangents.Length;
        int uv0Length = skinnedMesh.uv.Length;
        uint indexLength = skinnedMesh.GetIndexCount(0);

        binaryWriter.Write(verticesLength);
        for (int j = 0; j < verticesLength; ++j)
        {
            binaryWriter.Write(skinnedMesh.vertices[j].x);
            binaryWriter.Write(skinnedMesh.vertices[j].y);
            binaryWriter.Write(skinnedMesh.vertices[j].z);
        }

        binaryWriter.Write(normalsLength);
        for (int j = 0; j < normalsLength; ++j)
        {
            binaryWriter.Write(skinnedMesh.normals[j].x);
            binaryWriter.Write(skinnedMesh.normals[j].y);
            binaryWriter.Write(skinnedMesh.normals[j].z);
        }

        binaryWriter.Write(tangentsLength);
        for (int j = 0; j < tangentsLength; ++j)
        {
            binaryWriter.Write(skinnedMesh.tangents[j].x);
            binaryWriter.Write(skinnedMesh.tangents[j].y);
            binaryWriter.Write(skinnedMesh.tangents[j].z);
            binaryWriter.Write(skinnedMesh.tangents[j].w);
        }

        binaryWriter.Write(uv0Length);
        for (int j = 0; j < uv0Length; ++j)
        {
            binaryWriter.Write(skinnedMesh.uv[j].x);
            binaryWriter.Write(skinnedMesh.uv[j].y);
        }

        binaryWriter.Write(indexLength);

        for (int j = 0; j < indexLength; ++j)
        {
            binaryWriter.Write(skinnedMesh.GetIndices(0)[j]);
        }

        //binaryWriter.Write('S');        // <SkinningInfo>
        binaryWriter.Write((skinnedMeshRenderer) ? string.Copy(skinnedMeshRenderer.name).Replace(" ", "_") : "null");
        binaryWriter.Write(' ');

        int nBonesPerVertex = (int)skinnedMeshRenderer.quality; //SkinQuality.Auto:0, SkinQuality.Bone1:1, SkinQuality.Bone2:2, SkinQuality.Bone4:4
        if (nBonesPerVertex == 0)
            nBonesPerVertex = 4;

        binaryWriter.Write('V');       // <BonesPerVertex>
        binaryWriter.Write(nBonesPerVertex);
        binaryWriter.Write('B');       // <Bounds>
        //binaryWriter.Write(skinnedMeshRenderer.localBounds.center.x);
        //binaryWriter.Write(skinnedMeshRenderer.localBounds.center.y);
        //binaryWriter.Write(skinnedMeshRenderer.localBounds.center.z);
        //binaryWriter.Write(skinnedMeshRenderer.localBounds.extents.x);
        //binaryWriter.Write(skinnedMeshRenderer.localBounds.extents.y);
        //binaryWriter.Write(skinnedMeshRenderer.localBounds.extents.z);
        binaryWriter.Write('N');       // <BoneNames>
        binaryWriter.Write(skinnedMeshRenderer.bones.Length);
        if (skinnedMeshRenderer.bones.Length > 0)
        {
            foreach (Transform bone in skinnedMeshRenderer.bones)
                binaryWriter.Write((bone.gameObject) ? string.Copy(bone.gameObject.name).Replace(" ", "_") : "null");
        }
        binaryWriter.Write('O');           // <BoneOffsets>
        binaryWriter.Write(skinnedMeshRenderer.sharedMesh.bindposes.Length);
        if (skinnedMeshRenderer.sharedMesh.bindposes.Length > 0)
        {
            foreach (Matrix4x4 matrix in skinnedMeshRenderer.sharedMesh.bindposes)
            {
                binaryWriter.Write(matrix.m00);
                binaryWriter.Write(matrix.m10);
                binaryWriter.Write(matrix.m20);
                binaryWriter.Write(matrix.m30);
                binaryWriter.Write(matrix.m01);
                binaryWriter.Write(matrix.m11);
                binaryWriter.Write(matrix.m21);
                binaryWriter.Write(matrix.m31);
                binaryWriter.Write(matrix.m02);
                binaryWriter.Write(matrix.m12);
                binaryWriter.Write(matrix.m22);
                binaryWriter.Write(matrix.m32);
                binaryWriter.Write(matrix.m03);
                binaryWriter.Write(matrix.m13);
                binaryWriter.Write(matrix.m23);
                binaryWriter.Write(matrix.m33);
            }
        }
        binaryWriter.Write('I');           // "<BoneIndices>: "
        binaryWriter.Write(skinnedMeshRenderer.sharedMesh.boneWeights.Length);
        if (skinnedMeshRenderer.sharedMesh.boneWeights.Length > 0)
        {
            foreach (BoneWeight boneWeight in skinnedMeshRenderer.sharedMesh.boneWeights)
            {
                binaryWriter.Write(boneWeight.boneIndex0);
                binaryWriter.Write(boneWeight.boneIndex0);
                binaryWriter.Write(boneWeight.boneIndex0);
                binaryWriter.Write(boneWeight.boneIndex0);
            }
        }
        binaryWriter.Write('W');               // <BoneWeights>
        binaryWriter.Write(skinnedMeshRenderer.sharedMesh.boneWeights.Length);
        if (skinnedMeshRenderer.sharedMesh.boneWeights.Length > 0)
        {
            foreach (BoneWeight boneWeight in skinnedMeshRenderer.sharedMesh.boneWeights)
            {
                binaryWriter.Write(boneWeight.weight0);
                binaryWriter.Write(boneWeight.weight1);
                binaryWriter.Write(boneWeight.weight2);
                binaryWriter.Write(boneWeight.weight3);
            }
        }
        binaryWriter.Write('E');                   // </SkinningInfo>
    }

    void WriteAnimationMatrixHierarchy(Transform current)
    {
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(current.localPosition, current.localRotation, current.localScale);

        animationBinaryWriter.Write(matrix.m00);
        animationBinaryWriter.Write(matrix.m10);
        animationBinaryWriter.Write(matrix.m20);
        animationBinaryWriter.Write(matrix.m30);
        animationBinaryWriter.Write(matrix.m01);
        animationBinaryWriter.Write(matrix.m11);
        animationBinaryWriter.Write(matrix.m21);
        animationBinaryWriter.Write(matrix.m31);
        animationBinaryWriter.Write(matrix.m02);
        animationBinaryWriter.Write(matrix.m12);
        animationBinaryWriter.Write(matrix.m22);
        animationBinaryWriter.Write(matrix.m32);
        animationBinaryWriter.Write(matrix.m03);
        animationBinaryWriter.Write(matrix.m13);
        animationBinaryWriter.Write(matrix.m23);
        animationBinaryWriter.Write(matrix.m33);

        if(current.childCount> 0)
        {
            for (int i = 0; i < current.childCount; ++i)
            {
                WriteAnimationMatrixHierarchy(current.GetChild(i));
            }
        }
    }

    void WriteAnimationTransforms(string str, int nKeyFrame, float fKeyFrameTime)
    {
        animationBinaryWriter.Write(str);
        animationBinaryWriter.Write(nKeyFrame);
        animationBinaryWriter.Write(fKeyFrameTime);

        WriteAnimationMatrixHierarchy(transform);
    }

    void WriteFrameNameHierarchy(Transform obj)
    {
        animationBinaryWriter.Write((obj.gameObject) ? string.Copy(obj.gameObject.name).Replace(" ", "_") : "null");
        //animationBinaryWriter.Write(' ');

        if (obj.childCount>0)
        {
            for(int i=0;i<obj.childCount; ++i)
                WriteFrameNameHierarchy(obj.GetChild(i));
        }
    }

    void WriteFrameNames(string str)
    {
        animationBinaryWriter.Write(str);
        animationBinaryWriter.Write(m_nFrames);

        WriteFrameNameHierarchy(transform);
    }

    void WriteAnimationClipsInfo()
    {
        animationBinaryWriter.Write("<AnimationSets>: ");
        animationBinaryWriter.Write(m_AnimationClips.Length);

        WriteFrameNames("<FrameNames>: ");

        for (int i = 0; i < m_AnimationClips.Length; ++i)
        {
            int nFramesPerSec = (int)m_AnimationClips[i].frameRate;
            int nKeyFrames = Mathf.CeilToInt(m_AnimationClips[i].length * nFramesPerSec);
            animationBinaryWriter.Write("<AnimationSet>: ");
            animationBinaryWriter.Write(i);
            animationBinaryWriter.Write((m_AnimationClips[i]) ? string.Copy(m_AnimationClips[i].name).Replace(" ", "_") : "null");
            animationBinaryWriter.Write(m_AnimationClips[i].length);
            animationBinaryWriter.Write(nFramesPerSec);
            animationBinaryWriter.Write(nKeyFrames);

            float fFrameRate = (1.0f / nFramesPerSec);
            float fKeyFrameTime = 0.0f;

            for (int j = 0; j < nKeyFrames; ++j)
            {
                m_AnimationClips[i].SampleAnimation(gameObject, fKeyFrameTime);

                WriteAnimationTransforms("<Transforms>: ", j, fKeyFrameTime);

                fKeyFrameTime += fFrameRate;
            }
        }

        animationBinaryWriter.Write("</AnimationSets> ");
    }

    // Start is called before the first frame update
    void Start()
    {
        // text file
        textWriter = File.CreateText("player model data.txt");

        WriteFrameHierarachyText(gameObject.transform);

        if (m_AnimationClips.Length > 0)
        {
            textWriter.Write('\n');
            textWriter.Write("<Animation>:");
            textWriter.Write('\n');
            WriteAnimationClipsInfoText();
            textWriter.Write("</Animation>");
        }

        textWriter.Flush();
        textWriter.Close();

        m_nFrames = 0;

        m_pSkinnedMeshRenderers = GetComponentsInChildren<SkinnedMeshRenderer>();
        for(int i=0;i<m_pSkinnedMeshRenderers.Length;i++)
        {
            m_pSkinnedMeshRenderers[i].forceMatrixRecalculationPerRender = true;
        }

        // binary file
        binaryWriter = new BinaryWriter(File.Open("player model data.bin", FileMode.Create));

        //binaryWriter.Write('H');
        WriteFrameHierarachy(gameObject.transform);
        //binaryWriter.Write("</Hierarchy>: ");

        binaryWriter.Flush();
        binaryWriter.Close();

        animationBinaryWriter = new BinaryWriter(File.Open("player animation data.bin", FileMode.Create));

        if (m_AnimationClips.Length > 0)
        {
            //binaryWriter.Write('A');
            WriteAnimationClipsInfo();
            //binaryWriter.Write("</Animation> ");
        }

        animationBinaryWriter.Flush();
        animationBinaryWriter.Close();
    }
}
