using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class Inventory : MonoBehaviour
{


    // Start is called before the first frame update
    void Start()
    {

    }

    // Update is called once per frame
    void Update()
    {

    }

    public delegate void InventoryEventDelegate(InventoryItem item);

    public InventoryEventDelegate ItemAdded;
    public InventoryEventDelegate ItemUsed;
    public InventoryEventDelegate ItemRemoved;




    List<InventoryItem> items = new List<InventoryItem>();

    public void addItem(InventoryItem item)
    {
        if (items.Contains(item))
        {
            return;
        }
        items.Add(item);
        item.onPickup();

        ItemAdded?.Invoke(item);

    }

    public void useItem(InventoryItem item)
    {
        if (!items.Contains(item))
        {
            Debug.Log($"You don’t have {item.itemName}!");
            return;
        }

        ItemUsed?.Invoke(item);
        
    }

    public void removeItem(InventoryItem item)
    {
        if (items.Contains(item))
        {
            items.Remove(item);
            ItemRemoved?.Invoke(item);
        }
        ItemRemoved?.Invoke(item);
    }
}

