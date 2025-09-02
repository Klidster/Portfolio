using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class InventoryItem : MonoBehaviour
{

    public string itemName;
    public Sprite itemImage;
    public AudioClip pickupSfx;

    public void onPickup()
    {
        gameObject.SetActive(false);
        AudioSource.PlayClipAtPoint(pickupSfx, Camera.main.transform.position, 0.8f);

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
