using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class StopRainOnDestroy : MonoBehaviour
{

    void OnDestroy()
    {

        foreach (var ps in FindObjectsOfType<ParticleSystem>())
        {
            if (ps.gameObject.CompareTag("Rain"))
            {
                ps.Stop();
            }
        }
    }
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
