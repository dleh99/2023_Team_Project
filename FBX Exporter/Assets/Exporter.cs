using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;

public class Exporter : MonoBehaviour
{
    const int fileMode = 0;

    Mesh ethanMesh = null;
    Mesh sunglassMesh = null;

    Vector3[] ethanVertices = null;
    Vector3[] ethanNormals= null;
    Vector2[] ethanUv0 = null;

    string ethanVerticesText = null;

    MeshRenderer[] ethanMeshRenderer = null;

    Material ethanMaterial = null;

    StreamWriter writer = null;

    // Start is called before the first frame update
    void Start()
    {
        MeshFilter[] meshFilters = this.gameObject.GetComponentsInChildren<MeshFilter>();
        ethanMeshRenderer=this.gameObject.GetComponentsInChildren<MeshRenderer>();

        ethanMesh = meshFilters[0].mesh;
        sunglassMesh = meshFilters[1].mesh;

        ethanVertices = ethanMesh.vertices;
        ethanNormals = ethanMesh.normals;
        ethanUv0 = ethanMesh.uv;

        ethanMaterial = ethanMeshRenderer[0].material;
        //ethanMaterial.color

        writer = File.CreateText("EthanFbx.txt");
        BinaryWriter binaryWriter = new BinaryWriter(File.Open("EthanFBX.model", FileMode.Create));

        ethanVerticesText = ethanVertices.Length.ToString();
        writer.WriteLine("vertex num : " + ethanVerticesText);

        binaryWriter.Write(ethanVertices.Length);
        binaryWriter.Write('*');

        writer.Write("vertices : ");
        for (int i = 0;i<ethanVertices.Length;i++)
        {
            ethanVerticesText = ethanVertices[i].ToString();
            writer.Write(ethanVerticesText);

            binaryWriter.Write(ethanVertices[i].x);
            binaryWriter.Write(ethanVertices[i].y);
            binaryWriter.Write(ethanVertices[i].z);
        }
        binaryWriter.Write('*');

        ethanVerticesText = ethanNormals.Length.ToString();
        writer.WriteLine("\n\n" + "normal num : " + ethanVerticesText);

        binaryWriter.Write(ethanNormals.Length);
        binaryWriter.Write('*');

        writer.Write("normals : ");
        for (int i = 0; i < ethanNormals.Length; i++)
        {
            ethanVerticesText = ethanNormals[i].ToString();
            writer.Write(ethanVerticesText);

            binaryWriter.Write(ethanNormals[i].x);
            binaryWriter.Write(ethanNormals[i].y);
            binaryWriter.Write(ethanNormals[i].z);
        }
        binaryWriter.Write('*');

        ethanVerticesText = ethanUv0.Length.ToString();
        writer.WriteLine("\n\n" + "uv0 num : " + ethanVerticesText);

        binaryWriter.Write(ethanUv0.Length);
        binaryWriter.Write('*');

        writer.Write("uv0 : ");
        for (int i = 0; i < ethanUv0.Length; i++)
        {
            ethanVerticesText = ethanUv0[i].ToString();
            writer.Write(ethanVerticesText);

            binaryWriter.Write(ethanUv0[i].x);
            binaryWriter.Write(ethanUv0[i].y);
        }
        binaryWriter.Write('*');

        writer.Close();
        binaryWriter.Close();
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
