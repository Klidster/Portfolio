using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class InventoryItemClickable : MonoBehaviour
{
    public Inventory inventory;

    public InventoryItem item;

    public float useRange = 2f;

    public void OnItemClicked()
    {

        Collider[] hits = Physics.OverlapSphere(
            Camera.main.transform.position,
            useRange,
            LayerMask.GetMask("Door")
        );


        if (hits.Length > 0)
        {
            Debug.Log($"Using {item.itemName} on door");
            inventory.useItem(item);
        }
        else
        {
            Debug.Log("No door in range to use this item");
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
