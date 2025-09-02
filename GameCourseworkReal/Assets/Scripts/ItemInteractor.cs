using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Inventory))]
public class ItemInteractor : MonoBehaviour
{
    public Inventory inventory;


    private void OnControllerColliderHit(ControllerColliderHit hit)
    {
        InventoryItem item = hit.gameObject.GetComponent<InventoryItem>();
        if (item != null)
        {
            Debug.Log("Item Picked up: " + item.itemName);
            inventory.addItem(item);
        }
    }

    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetKeyDown(KeyCode.I))
        {
            if (Cursor.lockState == CursorLockMode.Locked)
                Cursor.lockState = CursorLockMode.Confined;
            else
                Cursor.lockState = CursorLockMode.Locked;
        }
    }
}
